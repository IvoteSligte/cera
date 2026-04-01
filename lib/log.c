#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

void println(const char *format, ...) {
  va_list args;
  va_start(args, format);

  vprintf(format, args);
  putchar('\n');

  va_end(args);
}

void eprintln(const char *format, ...) {
  va_list args;
  va_start(args, format);

  vfprintf(stderr, format, args);
  fputc('\n', stderr);
  
  va_end(args);
}

void eprintf(const char *format, ...) {
  va_list args;
  va_start(args, format);

  vfprintf(stderr, format, args);

  va_end(args);
}

void warn(const char *format, ...) {
  va_list args;
  va_start(args, format);

  fprintf(stderr, "WARN: ");
  vfprintf(stderr, format, args);

  va_end(args);
}

void panic(const char *format, ...) {
  va_list args;
  va_start(args, format);

  fprintf(stderr, "WARN: ");
  vfprintf(stderr, format, args);

  va_end(args);
  exit(1);
}
