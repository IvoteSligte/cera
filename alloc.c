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

void la_free_all(ListAllocator *allocator) {
  la_shrink(allocator, 0);
  free(allocator->data);
  allocator->data = NULL;
}

void *ra_calloc(RandomAllocator *allocator, size_t size) {
  allocator->data =
      realloc(allocator->data, sizeof(Memory) * (allocator->length + 1));
  Memory memory = {.ptr = calloc(1, size), .size = size};
  allocator->data[allocator->length] = memory;
  allocator->length++;
  return memory.ptr;
}

void *ra_recalloc(RandomAllocator *allocator, void *ptr, size_t new_size) {
  if (ptr == NULL) {
    return ra_calloc(allocator, new_size);
  }
  for (size_t i = 0; i < allocator->length; i++) {
    Memory *memory = &allocator->data[i];
    if (memory->ptr == ptr) {
      size_t old_size = memory->size;
      ptr = realloc(ptr, new_size);
      *memory = (Memory){.ptr = ptr, .size = new_size};
      if (new_size > old_size) {
        memset(ptr + old_size, 0, new_size - old_size);
      }
      return ptr;
    }
  }
  return NULL;
}

void ra_free_all(RandomAllocator *allocator) {
  for (size_t i = 0; i < allocator->length; i++) {
    free(allocator->data[i].ptr);
  }
  free(allocator->data);
  allocator->length = 0;
}
