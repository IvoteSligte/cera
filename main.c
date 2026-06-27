
#include "lib/api.h"

#ifdef __linux__
// used for closing of the mkstemps temporary file
#include <unistd.h>
#endif

// --- CLI args ---

#define REQUIRED_ARGS                                                          \
  REQUIRED_STRING_ARG(input_file, "input file", "Input file.")

#include "dep/argparse.h"

// --- end CLI args ---

#define CLEAN_OBJECT_FILE                                                      \
  if (object_file_desc != 0) {                                                 \
    close(object_file_desc);                                                   \
    remove(object_file);                                                       \
  }

static const char *const USAGES[] = {"cerac [options] [[--] args]",
                                     "cerac [options]", NULL};

typedef struct {
  bool run;
  const char *output_file;
  const char *package_type;
  const char **input_files;
  int num_input_files;
} Args;

Args parse_args(int argc, const char *argv[]) {
  Args args = {0};
  // short_name, long_name, value, help, callback, data, flags
  struct argparse_option options[] = {
      OPT_HELP(),
      OPT_BOOLEAN('r', "run", &args.run,
                  "Run the code instead of emitting an object file.", NULL, 0,
                  0),
      OPT_STRING('o', "output", &args.output_file, "Output file.", NULL, 0, 0),
      OPT_STRING('t', "package-type", &args.package_type,
                 "The type of package the compiler should emit. Either bin or "
                 "lib. (default: lib)",
                 NULL, 0, 0),
      OPT_END(),
  };
  struct argparse argparse;
  argparse_init(&argparse, options, USAGES, 0);
  argparse_describe(&argparse, "Compiler for the Cera programming language.",
                    "");
  args.num_input_files = argparse_parse(&argparse, argc, argv);
  args.input_files = argv;
  return args;
}

int main(int argc, const char *argv[]) {
  Args args = parse_args(argc, argv);

  if (!args.run && strcmp(args.output_file, "") == 0) {
    eprintf("--run requires argument: --output <output file>\n");
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
  if (args.num_input_files != 1) {
    panicf("TODO: multiple input files and Cera / object file mix.");
  }
  char *source = read_file(args.input_files[0]);
  if (source == NULL) {
    return 1;
  }
  char object_file_template[] = "/tmp/cera-XXXXXX.o";
  char *object_file = (char *)args.output_file;
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
