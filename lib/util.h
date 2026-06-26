#pragma once

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

__attribute__((noreturn)) void backtrace_abort(void);

// Reads a file to a string, returning NULL and printing the error on failure.
char *read_file(const char *path) __attribute__((nonnull));

// Reads an open file to a string, returning NULL and printing the error on
// failure. Starts reading at the file pointer and does not reset the pointer or
// close the filewhen done.
char *read_open_file(FILE *fptr, const char *path) __attribute__((nonnull));

#define eprintf(format, ...) fprintf(stderr, format __VA_OPT__(, ) __VA_ARGS__)
#define panicf(format, ...)                                                    \
  {                                                                            \
    eprintf("Panic: " format "\n" __VA_OPT__(, ) __VA_ARGS__);                 \
    backtrace_abort();                                                         \
  }
// Prints a message to stderr followed by a system error message.
#define pprintf($format, $args...)                                             \
  {                                                                            \
    eprintf($format, $args);                                                   \
    perror(" Error");                                                          \
  }

// Creates a string like sprintf, but panics on failure.
char *ssprintf(const char *fmt, ...);

#define assert_or($condition, $on_failure)                                     \
  if (!($condition)) {                                                         \
    $on_failure;                                                               \
    panicf("assertion '" #$condition "' failed");                              \
  }

// Compares strings for equality.
bool str_eq(const char *left, const char *right);

#define UNUSED(x) (void)(x)

#define _GET_MACRO(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, NAME,    \
                   ...)                                                        \
  NAME

#define _IS_ONE_OF_1(x, a) ((x) == (a))
#define _IS_ONE_OF_2(x, a, ...) ((x) == (a) || _IS_ONE_OF_1(x, __VA_ARGS__))
#define _IS_ONE_OF_3(x, a, ...) ((x) == (a) || _IS_ONE_OF_2(x, __VA_ARGS__))
#define _IS_ONE_OF_4(x, a, ...) ((x) == (a) || _IS_ONE_OF_3(x, __VA_ARGS__))
#define _IS_ONE_OF_5(x, a, ...) ((x) == (a) || _IS_ONE_OF_4(x, __VA_ARGS__))
#define _IS_ONE_OF_6(x, a, ...) ((x) == (a) || _IS_ONE_OF_5(x, __VA_ARGS__))
#define _IS_ONE_OF_7(x, a, ...) ((x) == (a) || _IS_ONE_OF_6(x, __VA_ARGS__))
#define _IS_ONE_OF_8(x, a, ...) ((x) == (a) || _IS_ONE_OF_7(x, __VA_ARGS__))
#define _IS_ONE_OF_9(x, a, ...) ((x) == (a) || _IS_ONE_OF_8(x, __VA_ARGS__))
#define _IS_ONE_OF_10(x, a, ...) ((x) == (a) || _IS_ONE_OF_9(x, __VA_ARGS__))
#define _IS_ONE_OF_11(x, a, ...) ((x) == (a) || _IS_ONE_OF_10(x, __VA_ARGS__))
#define _IS_ONE_OF_12(x, a, ...) ((x) == (a) || _IS_ONE_OF_11(x, __VA_ARGS__))

#define IS_ONE_OF(x, ...)                                                      \
  _GET_MACRO(__VA_ARGS__, _IS_ONE_OF_12, _IS_ONE_OF_11, _IS_ONE_OF_10,         \
             _IS_ONE_OF_9, _IS_ONE_OF_8, _IS_ONE_OF_7, _IS_ONE_OF_6,           \
             _IS_ONE_OF_5, _IS_ONE_OF_4, _IS_ONE_OF_3, _IS_ONE_OF_2,           \
             _IS_ONE_OF_1)(x, __VA_ARGS__)

#define SWAP(a, b)                                                             \
  {                                                                            \
    __auto_type t = *a;                                                        \
    *a = *b;                                                                   \
    *b = t;                                                                    \
  }

#define ARRAY_LENGTH(array) (sizeof(array) / sizeof((array)[0]))

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#define FMT($string) (int)($string).length, ($string).text

#define FILE_EXTENSION ".ce"
