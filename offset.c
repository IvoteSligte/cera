#include "offset.h"

OffsetInfo get_offset_info(const char *source, size_t offset) {
  OffsetInfo info = {0};
  info.line = source;

  for (size_t i = 0; i < offset; i++) {
    if (source[i] == '\n') {
      info.line = &source[i];
      info.line_number++;
    } else {
      info.column_number++;
    }
  }
  info.line_length = info.column_number;
  for (size_t i = offset; source[i] != '\n' && source[i] != '\0'; i++) {
    info.line_length++;
  }
  return info;
}
