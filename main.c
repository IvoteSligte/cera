
#include "lib/api.h"

#ifdef __linux__
// used for closing of the mkstemps temporary file
#include <unistd.h>
#endif

// --- CLI args ---

// FIXME: currently required (i.e. positional) arguments must come before
// boolean flags and optional arguments. Ideally, mixing them should be
// supported, as well as `program -- --escaped-argument`
#define REQUIRED_ARGS                                                          \
  REQUIRED_STRING_ARG(input_file, "input file", "Input file.")

// TODO: --output argument shorthand -o
#define OPTIONAL_ARGS                                                          \
  OPTIONAL_STRING_ARG(output_file, "", "--output", "output file",              \
                      "Output file.")                                          \
  OPTIONAL_STRING_ARG(                                                         \
      package_type, "lib", "--package-type", "package type",                   \
      "The type of package the compiler should emit. Either bin or lib.")

#define BOOLEAN_ARGS                                                           \
  BOOLEAN_ARG(run, "-r", "Run the code instead of emitting an object file.")   \
  BOOLEAN_ARG(help, "-h", "Show this menu.")

#include "includes/easyargs.h"

// --- end CLI args ---

#define CLEAN_OBJECT_FILE                                                      \
  if (object_file_desc != 0) {                                                 \
    close(object_file_desc);                                                   \
    remove(object_file);                                                       \
  }

int main(int argc, char *argv[]) {
  args_t args = make_default_args();

  if (!parse_args(argc, argv, &args) || args.help) {
    print_help(argv[0]);
    return 1;
  }
  if (!args.run && strcmp(args.output_file, "") == 0) {
    eprintf("Missing argument: --output <output file>\n");
    return 1;
  }
  bool is_binary = false;
  if (strcmp(args.package_type, "lib") == 0) {
  } else if (strcmp(args.package_type, "bin") == 0) {
    is_binary = true;
  } else {
    eprintf("package-type should be either lib or bin.\n");
    return 1;
  }
  char *source = read_file(args.input_file);
  if (source == NULL) {
    return 1;
  }
  char object_file_template[] = "/tmp/cera-XXXXXX.o";
  char *object_file = args.output_file;
  int object_file_desc = 0;
  if (is_binary) {
    // mkstemps replaces XXXXXX in object_file with a unique ID for the file
    // TODO: cross-platform
    if ((object_file_desc = mkstemps(object_file_template, 2)) == -1) {
      pprintf("Failed to create temporary file for compilation.");
      free(object_file);
      free(source);
      return 1;
    }
    object_file = object_file_template;
  }
  CompileErrors errors = args.run ? compile_and_run(source)
                                  : compile_to_object_file(source, object_file);
  if (errors.length > 0) {
    print_compile_errors(errors);
    free_compile_errors(&errors);
    CLEAN_OBJECT_FILE;
    free(source);
    return 1;
  }
  if (is_binary) {
    if (!link_to_binary(object_file, args.output_file)) {
      CLEAN_OBJECT_FILE;
      free(source);
      return 1;
    }
  }
  CLEAN_OBJECT_FILE;
  free(source);
  return 0;
}
