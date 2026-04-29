#include "util.h"

#ifdef __linux__

#include <execinfo.h> // for backtrace
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


char *read_file(const char *path) {
  FILE *fptr = fopen(path, "r");

  if (fptr == NULL) {
    perror("Failed to open file:");
    return NULL;
  }
  if (fseek(fptr, 0L, SEEK_END) != 0) {
    perror("Failed to seek to end of file:");
    return NULL;
  }
  size_t size = ftell(fptr);
  if (fseek(fptr, 0L, SEEK_SET) != 0) {
    perror("Failed to seek to start of file:");
    return NULL;
  }
  char *data = malloc(size + 1);

  if (fread(data, 1, size, fptr) != size) {
    perror("Reading file returned an unexpected number of bytes:");
    free(data);
    return NULL;
  }
  data[size] = '\0';

  if (fclose(fptr) != 0) {
    perror("Failed to close file:");
  }
  return data;
}

