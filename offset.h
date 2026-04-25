#pragma once

#include <stddef.h>

typedef struct {
  const char *line;
  int line_length;
  int column_number;
  size_t line_number;
} OffsetInfo;

OffsetInfo get_offset_info(const char *source, size_t offset);
