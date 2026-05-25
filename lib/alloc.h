#pragma once

#include "alloc.h"
#include "util.h"

#include <stdint.h>

// Allocator that tracks its allocations in a list.
// Allows shrinking the list to deallocate a number of recent allocations at
// once.
// Guarantees that the allocated data is never moved.
typedef struct ListAllocator {
  void **data;
  size_t length;
} ListAllocator;

void *la_calloc(ListAllocator *allocator, size_t size);

// Resizes the memory associated with PTR to SIZE bytes.
// Does not fill added space with zeroes.
// Returns NULL if the memory associated with PTR was not allocated with this
// allocator.
void *la_realloc(ListAllocator *allocator, void *ptr, size_t new_size);

void la_shrink(ListAllocator *allocator, size_t new_length);

void la_free_all(ListAllocator *allocator);

// Guarantees that the allocated data is never moved (unless ra_realloc is
// called).
typedef struct {
  void *ptr;
  size_t size;
} Memory;

typedef struct RandomAllocator {
  Memory *data;
  size_t length;
} RandomAllocator;

void *ra_calloc(RandomAllocator *allocator, size_t size);

// Resizes the memory associated with PTR to SIZE bytes.
// May move the memory.
// Calls ra_calloc if PTR == NULL.
// Returns NULL if PTR was not allocated by this allocator.
void *ra_recalloc(RandomAllocator *allocator, void *ptr, size_t new_size);

void ra_free_all(RandomAllocator *allocator);

// An allocator that grows like a stack.
// Does not keep track of individual allocations, so reallocation is not possible.
// Guarantees that the allocated data is never moved.
typedef struct StackAllocator {
  // Pointer to the current chunk.
  void *current;
  // Offset of the first free byte in the current chunk.
  // May also be exactly CHUNK_SIZE.
  size_t offset;

  // List of all allocated chunks.
  void **chunks;
  size_t length;
} StackAllocator;

void *sa_calloc(StackAllocator *allocator, size_t size);
void sa_shrink(StackAllocator *allocator, size_t size);
void sa_free_all(StackAllocator *allocator);
