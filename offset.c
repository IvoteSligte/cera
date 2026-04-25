#include "offset.h"

#include <assert.h>
#include <string.h>

OffsetInfo get_offset_info(const char *source, size_t offset) {
  assert(offset < strlen(source));

  OffsetInfo info = {0};
  info.line = source;
  info.line_number = 1; // line numbers start at 1

  for (size_t i = 0; i < offset; i++) {
    if (source[i] == '\n') {
      info.line = &source[i + 1];
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
