#define _GNU_SOURCE
#include "rlmalloc.h"

#include <unistd.h> //sbrk
#include <stdio.h> //printf
#include <stdbool.h>

static void* heap_ptr=NULL;

typedef struct block_metadata* block_metadata_ptr;

/**
 *  Block metadata
 */
struct block_metadata {
  size_t size;
  block_metadata_ptr next;
  bool is_free;
};

#define BLOCK_SIZE sizeof(struct block_metadata)

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
  block_metadata_ptr block_runner = heap_ptr;
  while (block_runner && !(is_suitable_block(block_runner, size))) {
    last = block_runner;
    block_runner = block_runner->next;
  }

  return block_runner;
}

block_metadata_ptr extend_heap(block_metadata_ptr last, size_t size) {
  block_metadata_ptr block;
  block = (block_metadata_ptr)sbrk(0);

  if (sbrk(BLOCK_SIZE + size) == (void*)-1) { // sbrk failed
    return NULL;
  }

  block->size = size;
  block->next = NULL;

  if (last != NULL) {
    last->next = block;
  }

  block->is_free = false;
  return block;
}

void* rlmalloc(size_t size) {
  printf("rlmalloc called. Size: %zu", size);
  void *p;
  p = sbrk (0);
  /* If sbrk fails , we return NULL */
  if (sbrk(size) == (void*) -1)
    return NULL;
  return p;
}

void rlfree(void* ptr) {
  // TODO
}
