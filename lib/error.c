#include "error.h"
#include "util.h"

typedef CompileError Error;
typedef CompileErrors Errors;

void print_compile_error(Error error) {
  eprintf("Error: %s\n", error.message);
  eprintf(">>> line %zu, column %zu\n", error.line, error.column);
  eprintf("\n");
}

void print_compile_errors(Errors errors) {
  eprintf("\n");
  for (size_t i = 0; i < errors.length; i++)
    print_compile_error(errors.data[i]);
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
