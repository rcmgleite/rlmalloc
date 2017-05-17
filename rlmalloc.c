#define _GNU_SOURCE
#include "rlmalloc.h"

#include <unistd.h> //sbrk
#include <stdio.h> //printf
#include <stdbool.h>

/**
 *  Global variable that holds a pointer to the end of the heap
 */
static void* heap_base_ptr=NULL;

/**
 *  Block metadata
 *    Is a double-linked list
 */
typedef struct block_metadata {
  size_t size;
  struct block_metadata* next;
  struct block_metadata* prev;
  bool is_free;
  void* validation_ptr;
  char block_metadata_end[0]; // http://gcc.gnu.org/onlinedocs/gcc/Zero-Length.html
} Block_metadata;

#define BLOCK_METADATA_SIZE sizeof(Block_metadata)

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
static inline bool is_suitable_block(Block_metadata* block, size_t size) {
  return block->is_free && block->size >= size;
}

/**
 *  Returns a block if it finds a suitable one or NULL otherwise
 *
 *  @param Block_metadata* last is used just to make it easy to extend
 *    the heap in case no suitable block is found
 */
Block_metadata* get_free_block(Block_metadata** last, size_t size) {
  Block_metadata* block_runner = (Block_metadata*)heap_base_ptr;
  while (block_runner && !(is_suitable_block(block_runner, size))) {
    *last = block_runner;
    block_runner = block_runner->next;
  }

  return block_runner;
}

/*
 *  Extends the heap using srbk like system calls
 * 
 *  @param Block_metadata* last: pointer to the last block allocated before a call to extend_heap
 *  @param size_t size: size of the requested new block
 */
Block_metadata* extend_heap(Block_metadata* last, size_t size) {
  Block_metadata* block;
  block = (Block_metadata*)sbrk(0);

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
 *  @parm Block_metadata* block: block to be splitted
 *  @size_t size new block size
 */
void split_block(Block_metadata* block, size_t size) {
  Block_metadata* new_block = (Block_metadata*) block->block_metadata_end + size; // important to have the block_metadata_end as char[0] for the pointer arithmetic
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
  Block_metadata* block_to_return = NULL;

  if (!heap_base_ptr) { // first allocation
    block_to_return = extend_heap(NULL, aligned_size);
    if (!block_to_return) {
      return NULL;
    }
    
    heap_base_ptr = block_to_return;
  } else {
    Block_metadata* last_heap_ptr = (Block_metadata*)heap_base_ptr;
    block_to_return = get_free_block(&last_heap_ptr, aligned_size);
    if (block_to_return) { // There is a free block already got from OS
      if (block_to_return->size - aligned_size >= BLOCK_METADATA_SIZE + sizeof(long)) {
        split_block(block_to_return, aligned_size);
      }
      
      block_to_return->is_free = false;
    } else { // No free blocks, we must ask more memory for the OS
      block_to_return = extend_heap(last_heap_ptr, aligned_size);
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
 *  @param Block_metadata* block
 */
Block_metadata* merge_block_with_next(Block_metadata* block) {
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
Block_metadata* get_block_metadata_from_ptr(void* ptr) {
  char* aux = (char*)ptr; // needed for the pointer arithmetic on the return statement
  return (Block_metadata*)(aux - BLOCK_METADATA_SIZE);
}

bool is_valid_ptr(void* ptr) {
  if (heap_base_ptr) {
    if (ptr > heap_base_ptr && ptr < sbrk(0)) { // heap range check
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
    printf("\nInvalid ptr passed to 'free'\n");
    return;
  }

  Block_metadata* block = get_block_metadata_from_ptr(ptr);
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
      heap_base_ptr = NULL;
    }

    brk(block);
  }
}
