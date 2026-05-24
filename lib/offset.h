#pragma once

#include <stddef.h>

typedef struct {
  const char *line;
  size_t line_length;
  size_t column_number;
  size_t line_number;
} OffsetInfo;

OffsetInfo get_offset_info(const char *source, size_t offset)
    __attribute__((nonnull));
