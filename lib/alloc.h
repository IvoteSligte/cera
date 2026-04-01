#pragma once

#include <stdlib.h>

typedef struct {
  void *(*allocate)(void **data, size_t size);
  void (*free)(void **data, void *memory);
  void (*free_all)(void **data);
  void *data;
} Allocator;

typedef struct {
  Allocator allocator;
} Context;

extern const Allocator DEFAULT_ALLOCATOR;
extern Context context;

// Allocates a number of zeroed bytes using the context's allocator. Returns
// NULL on failure.
void *try_allocate(size_t size);

// Allocates a number of zeroed bytes using the context's allocator. Panics on
// failure.
void *allocate(size_t size);

// Frees the memory associated with the given pointer from the context's
// allocator. Panics on failure.
void free_memory(void *memory);

// Frees all the memory allocated using the context's allocator.
void free_all(void);

#define alloc(type) allocate(sizeof(type))
