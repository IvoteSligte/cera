#include "alloc.h"

// --- list allocator ---

void *la_calloc(ListAllocator *allocator, size_t size) {
  allocator->data =
      realloc(allocator->data, sizeof(void *) * (allocator->length + 1));
  void *memory = calloc(1, size);
  allocator->data[allocator->length] = memory;
  allocator->length++;
  return memory;
}

void *la_realloc(ListAllocator *allocator, void *ptr, size_t new_size) {
  if (ptr == NULL) {
    return la_calloc(allocator, new_size);
  }
  for (size_t i = 0; i < allocator->length; i++) {
    if (allocator->data[i] == ptr) {
      ptr = realloc(ptr, new_size);
      allocator->data[i] = ptr;
      return ptr;
    }
  }
  return NULL;
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
  *allocator = (ListAllocator){0};
}

// --- random allocator ---

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
  *allocator = (RandomAllocator){0};
}

// --- stack allocator ---

const size_t CHUNK_SIZE = 1 << 16;

static void *sa_add_chunk(StackAllocator *allocator) {
  allocator->chunks =
      realloc(allocator->chunks, sizeof(void *) * (allocator->length + 1));
  allocator->current = calloc(CHUNK_SIZE, 1);
  allocator->chunks[allocator->length] = allocator->current;
  allocator->length += 1;
  allocator->offset = 0;
  return allocator->current;
}

void *sa_calloc(StackAllocator *allocator, size_t size) {
  if (allocator->current == NULL || allocator->offset + size > CHUNK_SIZE) {
    assert(size <= CHUNK_SIZE);
    return sa_add_chunk(allocator);
  }
  allocator->offset = allocator->offset + size;
  return allocator->current + allocator->offset;
}

void sa_shrink(StackAllocator *allocator, size_t size) {
  while (size > allocator->offset) {
    free(allocator->current);
    size -= allocator->offset;
    assert(allocator->length > 0);
    allocator->length -= 1;
    allocator->current = allocator->chunks[allocator->length];
  }
  allocator->offset -= size;
}

void sa_free_all(StackAllocator *allocator) {
  for (size_t i = 0; i < allocator->length; i++) {
    free(allocator->chunks[i]);
  }
  free(allocator->chunks);
  *allocator = (StackAllocator){0};
}
