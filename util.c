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
