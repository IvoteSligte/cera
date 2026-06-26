
#include "lib/analyzer.h"
#include "lib/api.h"

int main(int argc, char *argv[]) {
  if (argc < 2) {
    eprintf("Usage: %s <file>\n", argv[0]);
    return 1;
  }
  const char *path = argv[1];
  char *source = read_file(path);
  if (source == NULL) {
    return 1;
  }

  CompileErrors errors = compile_and_run(source);
  if (errors.length > 0) {
    free_compile_errors(&errors);
    return false;
  }
  free(source);
  return 0;
}
