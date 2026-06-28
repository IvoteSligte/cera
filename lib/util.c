#include "util.h"
#include <stdarg.h>

#ifdef __linux__

#include <execinfo.h> // for backtrace
#include <sys/wait.h>
#include <unistd.h>

#define MAX_FRAMES 64

// Prints a backtrace on Linux. Does nothing on other platforms.
void print_backtrace(void) {
  void *buffer[MAX_FRAMES];
  int nptrs = backtrace(buffer, MAX_FRAMES);

  eprintf("Backtrace:\n");

  char **symbols = backtrace_symbols(buffer, nptrs);

  // skip the first 3 frames as they correspond to debug functions themselves
  for (int i = 3; i < nptrs; i++) {
    // Resolve file + line using addr2line
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "addr2line -f -p -e /proc/%d/exe %p", getpid(),
             buffer[i]);

    FILE *fp = popen(cmd, "r");
    if (fp != NULL) {
      char line[512];
      if (fgets(line, sizeof(line), fp)) {
        eprintf("- %s", line);
      }
      pclose(fp);
    }
  }

  free(symbols);
}

void backtrace_abort(void) {
  print_backtrace();
  abort();
}

#else

void backtrace_abort(void) { abort(); }

#endif

char *read_open_file(FILE *fptr, const char *path) {
  assert(fptr != NULL);
  assert(path != NULL);
  if (fseek(fptr, 0L, SEEK_END) != 0) {
    pprintf("Failed to seek to end of file `%s`.", path);
    return NULL;
  }
  size_t size = ftell(fptr);
  if (fseek(fptr, 0L, SEEK_SET) != 0) {
    pprintf("Failed to seek to start of file `%s`.", path);
    return NULL;
  }
  char *data = malloc(size + 1);

  if (fread(data, 1, size, fptr) != size) {
    pprintf("Reading file `%s` returned an unexpected number of bytes.", path);
    free(data);
    return NULL;
  }
  data[size] = '\0';
  return data;
}

char *read_file(const char *path) {
  FILE *fptr = fopen(path, "r");

  if (fptr == NULL) {
    pprintf("Failed to open file `%s`.", path);
    return NULL;
  }
  char *data = read_open_file(fptr, path);
  if (fclose(fptr) != 0) {
    pprintf("Failed to close file `%s`.", path);
  }
  return data;
}

char *ssprintf(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  char *out = NULL;
  // TODO: non-GNU alternative to asprintf
  if (vasprintf(&out, fmt, args) < 0)
    panicf("Failed to ssprintf.");
  return out;
}

bool str_eq(const char *left, const char *right) {
  return strcmp(left, right) == 0;
}

bool str_ends_with(const char *s, const char *suffix) {
  size_t s_len = strlen(s);
  size_t suffix_len = strlen(suffix);
  if (s_len < suffix_len)
    return false;
  return str_eq(&s[s_len - suffix_len], suffix);
}

void str_array_remove(const char **array, size_t *length, size_t rm_index) {
  assert(*length > 0);
  *length -= 1;
  for (size_t i = rm_index; i < *length; i++) {
    array[i] = array[i + 1];
  }
}

bool create_temp_file(char *file_name_template) {
  // TODO: cross-platform
  const char *xxxxxx = strstr(file_name_template, "XXXXXX");
  assert(xxxxxx != NULL);
  size_t suffix_len = strchr(file_name_template, '\0') - (xxxxxx + 6);
  int file_descriptor = mkstemps(file_name_template, suffix_len);
  if (file_descriptor == -1) {
    pprintf("Failed to create temporary file.");
    return false;
  }
  close(file_descriptor); // file still exists after closing
  return true;
}

int run_command(const char *const argv[]) {
  // TODO: cross-platform
  pid_t pid = fork();
  if (pid == 0) {
    execvp(argv[0], (char **)argv); // can fail if cc is not in PATH
    pprintf("Failed to execute command.");
    exit(1); // kill child process
  }
  int status = 0;
  if (waitpid(pid, &status, 0) == -1) {
    pprintf("Failed to wait for command execution to finish.");
    return 1;
  }
  return WEXITSTATUS(status);
}
