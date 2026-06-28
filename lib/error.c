#include "error.h"
#include "offset.h"
#include "util.h"

typedef CompileError Error;
typedef CompileErrors Errors;

void print_compile_error(const char *source, Error error) {
  OffsetInfo oi = get_offset_info(source, error.offset);

  eprintf("Error: %s\n", error.message);
  eprintf(">>> line %zu, column %zu\n", error.line, error.column);
  eprintf(" | %.*s\n", (int)oi.line_length, oi.line);
  eprintf(" | %*s", (int)oi.column_number, " ");
  size_t length = MIN(error.length, oi.line_length - oi.column_number);
  for (size_t i = 0; i < length; i++)
    eprintf("~");
  eprintf("\n");
}

void print_compile_errors(const char *source, Errors errors) {
  eprintf("\n");
  for (size_t i = 0; i < errors.length; i++)
    print_compile_error(source, errors.data[i]);
}

void free_compile_error(Error *error) {
  free(error->message);
  *error = (Error){0};
}

void free_compile_errors(Errors *errors) {
  for (size_t i = 0; i < errors->length; i++)
    free_compile_error(&errors->data[i]);
  free(errors->data);
  *errors = (Errors){0};
}
