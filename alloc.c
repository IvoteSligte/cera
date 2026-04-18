#include "alloc.h"

void *la_calloc(ListAllocator *allocator, size_t size) {
  allocator->data =
      realloc(allocator->data, sizeof(void *) * (allocator->length + 1));
  void *memory = calloc(1, size);
  allocator->data[allocator->length] = memory;
  allocator->length++;
  return memory;
}

void la_shrink(ListAllocator *allocator, size_t new_length) {
  assert(new_length <= allocator->length);
  for (size_t i = new_length; i < allocator->length; i++) {
    free(allocator->data[i]);
  }
  allocator->length = new_length;
}

void la_free_all(ListAllocator *allocator) { la_shrink(allocator, 0); }

void *ra_calloc(RandomAllocator *allocator, size_t size) {
  allocator->data =
      realloc(allocator->data, sizeof(void *) * (allocator->length + 1));
  void *memory = calloc(1, size);
  allocator->data[allocator->length] = memory;
  allocator->length++;
  return memory;
}

void *ra_realloc(RandomAllocator *allocator, void *memory, size_t size) {
  if (memory == NULL) {
    return ra_calloc(allocator, size);
  }
  for (size_t i = 0; i < allocator->length; i++) {
    if (allocator->data[i] == memory) {
      allocator->data[i] = realloc(allocator->data[i], size);
      return allocator->data[i];
    }
  }
  return NULL;
}

void ra_free_all(RandomAllocator *allocator) {
  for (size_t i = 0; i < allocator->length; i++) {
    free(allocator->data[i]);
  }
}
