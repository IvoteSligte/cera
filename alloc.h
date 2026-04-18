#pragma once

#include "util.h"

// Allocator that tracks its allocations in a list.
// Allows shrinking the list to deallocate a number of recent allocations at
// once.
// Guarantees that the allocated data is never moved.
typedef struct {
  void **data;
  size_t length;
} ListAllocator;

void *la_calloc(ListAllocator *allocator, size_t size);
void la_shrink(ListAllocator *allocator, size_t new_length);
void la_free_all(ListAllocator *allocator);

// Guarantees that the allocated data is never moved (unless ra_realloc is called).
typedef struct {
  void **data;
  size_t length;
} RandomAllocator;

void *ra_calloc(RandomAllocator *allocator, size_t size);

// Resizes the memory associated with MEMORY to SIZE bytes.
// May move the memory.
// Calls ra_calloc if MEMORY == NULL.
// Returns NULL if MEMORY was not allocated by this allocator.
void *ra_realloc(RandomAllocator *allocator, void *memory, size_t size);

void ra_free_all(RandomAllocator *allocator);
