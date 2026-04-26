#pragma once

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "alloc.h"

__attribute__((noreturn))
void backtrace_abort(void);

#define eprintf(format, ...) fprintf(stderr, format __VA_OPT__(, ) __VA_ARGS__)
#define panicf(format, ...)                                                    \
  {                                                                            \
    eprintf("Panic: " format "\n" __VA_OPT__(, ) __VA_ARGS__);                 \
    backtrace_abort();                                                         \
  }

#define UNUSED(x) (void)(x)

// Argument counter (supports up to 5 here)
#define _GET_MACRO(_1, _2, _3, _4, _5, NAME, ...) NAME

#define _IS_ONE_OF_1(x, a) ((x) == (a))
#define _IS_ONE_OF_2(x, a, ...) ((x) == (a) || _IS_ONE_OF_1(x, __VA_ARGS__))
#define _IS_ONE_OF_3(x, a, ...) ((x) == (a) || _IS_ONE_OF_2(x, __VA_ARGS__))
#define _IS_ONE_OF_4(x, a, ...) ((x) == (a) || _IS_ONE_OF_3(x, __VA_ARGS__))
#define _IS_ONE_OF_5(x, a, ...) ((x) == (a) || _IS_ONE_OF_4(x, __VA_ARGS__))

#define IS_ONE_OF(x, ...)                                                      \
  _GET_MACRO(__VA_ARGS__, _IS_ONE_OF_5, _IS_ONE_OF_4, _IS_ONE_OF_3,            \
             _IS_ONE_OF_2, _IS_ONE_OF_1)(x, __VA_ARGS__)

#define SWAP(a, b)                                                             \
  {                                                                            \
    __auto_type t = *a;                                                        \
    *a = *b;                                                                   \
    *b = t;                                                                    \
  }

#define ARRAY_LENGTH(array) (sizeof(array) / sizeof((array)[0]))

#define MIN(a, b) ((a) < (b) ? (a) : (b))

