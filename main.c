
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

static const char *const USAGES[] = {"cerac [options] [[--] args]",
                                     "cerac [options]", NULL};

typedef struct {
  bool compile_only;
  const char *output_file;
  const char *code_file; // nullable
  const char **linker_files;
  size_t num_linker_files;
} Args;

// Modifies argv.
Args parse_args(int argc, const char *argv[]) {
  Args args = {0};
  // short_name, long_name, value, help, callback, data, flags
  struct argparse_option options[] = {
      OPT_HELP(),
      OPT_BOOLEAN(
          'c', "compile-only", &args.compile_only,
          "Only compile to an object file instead of to an executable binary.",
          NULL, 0, 0),
      OPT_STRING('o', "output", &args.output_file, "Output file.", NULL, 0, 0),
      OPT_END(),
  };
  struct argparse argparse;
  argparse_init(&argparse, options, USAGES, 0);
  argparse_describe(&argparse, "Compiler for the Cera programming language.",
                    "");

  args.num_linker_files = argparse_parse(&argparse, argc, argv);
  args.linker_files = argv;
  if (args.num_linker_files == 0) {
    eprintf("Missing input files. Use --help for more information.\n");
    exit(1);
  }
  for (size_t i = 0; i < args.num_linker_files;) {
    const char *file = args.linker_files[i];

    if (str_ends_with(file, FILE_EXTENSION)) {
      if (args.code_file != NULL) {
        // The compiler automatically discovers (implementation) files in a
        // project, unlike C compilers.
        // The compiler should be invoked once using --compile-only per library
        // to compile, which allows linking the object files with another Cera
        // package to produce a final binary.
        eprintf("Only one Cera file may be provided as input.\n");
        exit(1);
      }
      args.code_file = file;

      // remove code_file from linker_files
      str_array_remove(args.linker_files, &args.num_linker_files, i);
      // do not increment i here because the next file to inspect was moved 1
      // down the list
      continue;
    }
    if (!str_ends_with(file, ".o") &&
        !str_ends_with(file, ".obj")) { // object file extensions
      // TODO: detect other common file extensions, such as for static
      // libraries.
      eprintf("%s: Unknown file extension. Forwarding file to linker instead "
              "of compiling.\n",
              file);
    }
    i++;
  }
  if (args.output_file == NULL) {
    eprintf("Missing output file. Use --help for more information.\n");
    exit(1);
  }
  // TODO: check if input files exist
  return args;
}

#define REMOVE_TEMP_FILE                                                       \
  if (object_file == temp_object_file) {                                       \
    remove(object_file);                                                       \
  }

int main(int argc, const char *argv[]) {
  Args args = parse_args(argc, argv);

  char temp_object_file[] = "/tmp/cera-XXXXXX.o";
  const char *object_file = args.output_file;

  if (args.code_file != NULL) {
    char *source = read_file(args.code_file);
    if (source == NULL) {
      return 1;
    }
    if (!args.compile_only) {
      object_file = temp_object_file;
      if (!create_temp_file(temp_object_file)) {
        free(source);
        return 1;
      }
    }
    CompileErrors errors = compile_to_object_file(source, object_file);
    if (errors.length > 0) {
      print_compile_errors(errors);
      free_compile_errors(&errors);
      REMOVE_TEMP_FILE;
      free(source);
      return 1;
    }
    free(source);
  }
  // never overflows because the code file was originally in the same char*
  // buffer
  args.linker_files[args.num_linker_files] = object_file;
  args.num_linker_files += 1;

  for (size_t i = 0; i < args.num_linker_files; i++) { // TEMP
    eprintf("linker file[%zu] = %s\n", i, args.linker_files[i]);
  }

  if (!args.compile_only &&
      !link_to_executable(args.linker_files, args.num_linker_files,
                          args.output_file)) {
    REMOVE_TEMP_FILE;
    return 1;
  }
  REMOVE_TEMP_FILE;
  return 0;
}
