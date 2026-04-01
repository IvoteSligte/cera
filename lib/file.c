#include <stdio.h>
#include <stdlib.h>

#include "alloc.h"
#include "log.h"
#include "file.h"

String read_file_to_string(const char* path) {
  FILE *file = fopen(path, "rb");
  if (!file) {
    panic("Failed to open file '%s'.", path);
  }

  // Go to end to get file size
  fseek(file, 0, SEEK_END);
  long length = ftell(file);
  rewind(file);

  if (length < 0) {
    fclose(file);
    panic("Failed to read file '%s'.", path);
  }
  char *buffer = (char*)allocate(length);

  // Read file into buffer
  size_t read_size = fread(buffer, 1, length, file);
  if (read_size != (size_t)length) {
    free(buffer);
    fclose(file);
    panic("Failed to read the expected length of file '%s'.", path);
  }
  fclose(file);
  return (String) {
    .len = (size_t)(length),
    .data = buffer,    
  };
}
