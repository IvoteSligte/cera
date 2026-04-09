#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define eprintf(format, ...) fprintf(stderr, format __VA_OPT__(,) __VA_ARGS__)
#define panicf(format, ...)                                                     \
  {                                                                            \
eprintf(format __VA_OPT__(,) __VA_ARGS__);                                              \
    abort();                                                                   \
  }
