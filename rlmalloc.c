#define _GNU_SOURCE
#include "rlmalloc.h"

#include <unistd.h> //sbrk
#include <stdio.h> //printf
#include <stdbool.h>

/**
 *  Global variable that holds a pointer to the end of the heap
 */
static void* heap_ptr=NULL;

typedef struct block_metadata* block_metadata_ptr;

/**
 *  Block metadata
 *    Is a double-linked list
 */
struct block_metadata {
  size_t size;
  block_metadata_ptr next;
  block_metadata_ptr prev;
  bool is_free;
  void* validation_ptr;
  char block_metadata_end[0]; // http://gcc.gnu.org/onlinedocs/gcc/Zero-Length.html
};

#define BLOCK_METADATA_SIZE sizeof(struct block_metadata)

/*
 *  All pointers returned by malloc will be aligned for long type.
 *    8 aligned for x86-64 and 4 for x86-32
 *
 *  TODO: find out why glibc malloc returns 16-byte aligned pointers
 *  (http://stackoverflow.com/a/3994235/3051060) 
 */
static inline size_t align_long(size_t size) {
  /* This alignment function was borrowed from redis zmalloc */
  if (size&(sizeof(long)-1)) {
    size += sizeof(long)-(size&(sizeof(long)-1));
  }

  return size;
}

/*
 *  a suitable block is one that is free and has at least the size we are asking for
 */
static inline bool is_suitable_block(block_metadata_ptr block, size_t size) {
  return block->is_free && block->size >= size;
}

/**
 *  Returns a block if it finds a suitable one or NULL otherwise
 *
 *  @param block_metadata_ptr last is used just to make it easy to extend
 *    the heap in case no suitable block is found
 */
block_metadata_ptr get_free_block(block_metadata_ptr last, size_t size) {
  block_metadata_ptr block_runner = (block_metadata_ptr)heap_ptr;
  while (block_runner && !(is_suitable_block(block_runner, size))) {
    last = block_runner;
    block_runner = block_runner->next;
  }

  return block_runner;
}

/*
 *  Extends the heap using srbk like system calls
 * 
 *  @param block_metadata_ptr last: pointer to the last block allocated before a call to extend_heap
 *  @param size_t size: size of the requested new block
 */
block_metadata_ptr extend_heap(block_metadata_ptr last, size_t size) {
  block_metadata_ptr block;
  block = (block_metadata_ptr)sbrk(0);

  if (sbrk(BLOCK_METADATA_SIZE + size) == (void*)-1) { // sbrk failed
    return NULL;
  }

  block->size = size;
  block->next = NULL;
  block->prev = last;
  block->validation_ptr = block->block_metadata_end;

  if (last != NULL) {
    last->next = block;
  }

  block->is_free = false;
  return block;
}

/**
 *  Splits @block to have the @size size and creates a new block with the remaining space
 *
 *  @parm block_metadata_ptr block: block to be splitted
 *  @size_t size new block size
 */
void split_block(block_metadata_ptr block, size_t size) {
  block_metadata_ptr new_block = (block_metadata_ptr) block->block_metadata_end + size; // important to have the block_metadata_end as char[0] for the pointer arithmetic
  new_block->size = block->size - size - BLOCK_METADATA_SIZE;
  new_block->next = block->next;
  new_block->prev = block;
  new_block->is_free = true;
  new_block->validation_ptr = new_block->block_metadata_end;
  block->size = size;
  block->next = new_block;

  if (new_block->next) {
    new_block->next->prev = new_block;
  }
}

/*
 *  Malloc implementation using 'first fit' algorithm
 */
void* rlmalloc(size_t size) {
  size_t aligned_size = align_long(size);
  block_metadata_ptr block_to_return = NULL;

  if (!heap_ptr) { // first allocation
    block_to_return = extend_heap(NULL, aligned_size);
    if (!block_to_return) {
      return NULL;
    }
    
    heap_ptr = block_to_return;
  } else {
    block_metadata_ptr last_heap_ptr = (block_metadata_ptr)heap_ptr;
    block_to_return = get_free_block(last_heap_ptr, aligned_size);
    if (block_to_return) { // There is a free block already got from OS
      if (block_to_return->size - aligned_size >= BLOCK_METADATA_SIZE + sizeof(long)) {
        split_block(block_to_return, aligned_size);
      }
      
      block_to_return->is_free = false;
    } else { // No free blocks, we must ask more memory for the OS
      block_to_return = extend_heap(heap_ptr, aligned_size);
      if (!block_to_return) {
        return NULL;
      }
    }
  }

  return block_to_return->block_metadata_end;
}

/*
 *  If a block is next to other empty blocks, merges them into one
 *    This is a way to minimize memory fragmentatio
 *
 *  @param block_metadata_ptr block
 */
block_metadata_ptr merge_block_with_next(block_metadata_ptr block) {
  if (block->next && block->next->is_free) {
    block->size += BLOCK_METADATA_SIZE + block->next->size;
    block->next = block->next->next;

    if (block->next) {
      block->next->prev = block;
    }
  }
  
  return block;
}

/*
 *  Given a @ptr, it returns a pointer the it's block_metadata
 *
 *  @param void* ptr
 */
block_metadata_ptr get_block_metadata_from_ptr(void* ptr) {
  char* aux = (char*)ptr; // needed for the pointer arithmetic on the return statement
  return (block_metadata_ptr)(aux - BLOCK_METADATA_SIZE);
}

bool is_valid_ptr(void* ptr) {
  if (heap_ptr) {
    if (ptr > heap_ptr && ptr < sbrk(0)) { // heap range check
      return (ptr == get_block_metadata_from_ptr(ptr)->validation_ptr); // using the validation_ptr we are able to validate @ptr
    }
  }

  return false;
}

/*
 *  Free implementation
 *
 *   It's mandatory that the free function can:
 *     1) Validate the input pointer (is it realy a malloc’ed pointer ?)
 *     2) Finde the meta-data pointer
 */
void rlfree(void* ptr) {
  if (!is_valid_ptr(ptr)) {
    printf("Invalid ptr passed to 'free'");
    return;
  }

  block_metadata_ptr block = get_block_metadata_from_ptr(ptr);
  block->is_free = true;

  if (block->prev && block->prev->is_free) {
    block = merge_block_with_next(block->prev);
  }

  if (block->next) {
    block = merge_block_with_next(block);
  } else { // last block of the heap
    if (block->prev) { // There are more blocks to the left
      block->prev->next = NULL;
    } else { // all heap is now back to the OS
      heap_ptr = NULL;
    }

    brk(block);
  }
}
