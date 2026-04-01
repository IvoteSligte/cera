#include "alloc.h"
#include "log.h"

void *try_allocate(size_t size) {
  return context.allocator.allocate(context.allocator.data, size);
}

void *allocate(size_t size) {
  void *ptr = try_allocate(size);
  if (ptr == NULL) {
    panic("Failed to allocate %d bytes.", size);
  }
  return ptr;
}

void free_memory(void *memory) {
  context.allocator.free(context.allocator.data, memory);
}

void free_all(void) { context.allocator.free_all(context.allocator.data); }

void *default_allocate(void **data, size_t size) { return calloc(size, 1); }

void default_free_all(void **data) {
  panic("Called free_all on the default allocator (malloc).");
}

void default_free(void **data, void *memory) { free(memory); }

const Allocator DEFAULT_ALLOCATOR = {
    .allocate = default_allocate,
    .free = default_free,    
    .free_all = default_free_all,
    .data = NULL,
};

Context context = {.allocator = DEFAULT_ALLOCATOR};
