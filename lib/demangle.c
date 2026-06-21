// based on https://github.com/rust-lang/rustc-demangle/

#include "ast.h"

#include <ctype.h>

typedef RandomAllocator Allocator;

static bool starts_with(Name s, const char *prefix) {
  size_t len = strlen(prefix);
  return s.length >= len && (strncmp(s.text, prefix, len) == 0);
}

static Name take(Name s, size_t start) {
  assert(s.length >= start);
  return (Name){.text = &s.text[start], .length = s.length - start};
}

// Returns a NULL Name on failure.
static Name strip_legacy_prefix(Name s) {
  if (starts_with(s, "ZN")) {
    // On Windows, dbghelp strips leading underscores, so we accept "R..."
    // form too.
    return take(s, 2);
  } else if (starts_with(s, "_ZN")) {
    return take(s, 3);
  } else if (starts_with(s, "__ZN")) {
    // On OSX, symbols are prefixed with an extra _
    return take(s, 4);
  } else {
    return (Name){0};
  }
}

static bool is_ascii(Name s) {
  for (size_t i = 0; i < s.length; i++) {
    if ((s.text[i] & 0x80) != 0) {
      return false;
    }
  }
  return true;
}

// Extracts the length of an identifier, returning 0 on error.
static size_t extract_ident_length(Name s, size_t *i) {
  size_t length = 0;
  for (; *i < s.length; *i += 1) {
    char c = s.text[*i];
    if (!isdigit(c)) {
      return length;
    }
    int d = c - '0';
    if (length > SIZE_MAX / 10) {
      return 0;
    }
    length *= 10;
    if (length > SIZE_MAX - d) {
      return 0;
    }
    length += d;
  }
  return 0;
}

static void append_sized(Allocator *allocator, Name *output, const char *str,
                         size_t len) {
  output->text =
      ra_recalloc(allocator, (char *)output->text, output->length + len);
  memcpy((char *)output->text + output->length, str, len);
  output->length += len;
}

static void append(Allocator *allocator, Name *output, const char *str) {
  append_sized(allocator, output, str, strlen(str));
}

#define WRITE($str) append(allocator, output, $str)
#define WRITE_SIZED($str, $len) append_sized(allocator, output, $str, $len)

#define ESC($escaped, $unescaped)                                              \
  if (starts_with(take(ident, i + 1), $escaped)) {                             \
    WRITE($unescaped);                                                         \
    i += end_i + 1 - i;                                                        \
    continue;                                                                  \
  }

static void rust_demangle_ident(Allocator *allocator, Name ident,
                                Name *output) {
  for (size_t i = 0; i < ident.length;) {
    char c = ident.text[i];
    if (c == '.') {
      if (i + 1 < ident.length && ident.text[i + 1] == '.') {
        WRITE("::");
        i += 2;
      } else {
        WRITE(".");
        i += 1;
      }
      continue;
    }
    if (c == '$') {
      if (ident.length - i < 4) {
        WRITE_SIZED(ident.text + i, ident.length - i);
        return;
      }
      size_t end_i = i + 1;
      for (; end_i < ident.length; end_i++) {
        if (ident.text[end_i] == '$') {
          break;
        }
      }
      if (end_i < ident.length) {
        // these `continue;` on match
        ESC("SP", "@");
        ESC("BP", "*");
        ESC("RF", "&");
        ESC("LT", "<");
        ESC("GT", ">");
        ESC("LP", "(");
        ESC("RP", ")");
        ESC("C", ",");
        // TODO: programmatically decode other unicode codepoints (hex-encoded)
        ESC("u20", " ");
        ESC("u2b", "+");
        ESC("u3d", "=");
        ESC("u50", "(");
        ESC("u51", ")");
        ESC("u5b", "[");
        ESC("u5d", "]");
        ESC("u7b", "{");
        ESC("u7d", "}");
      }
      WRITE_SIZED(ident.text + i, end_i + 1 - i);
      i += end_i + 1 - i;
      continue;
    }
    WRITE_SIZED(ident.text + i, 1);
    i += 1;
  }
}

static bool rust_demangle_legacy(Allocator *allocator, Name m, Name *output) {
  // First validate the symbol. If it doesn't look like anything we're
  // expecting, we just print it literally. Note that we must handle non-Rust
  // symbols because we could have any function in the backtrace.
  Name inner = strip_legacy_prefix(m);

  // Paths always start with uppercase characters. We only work with ascii text.
  if (inner.length == 0 || !is_ascii(inner)) {
    return false;
  }
  *output = (Name){0};
  for (size_t i = 0, element_i = 0;; element_i++) {
    if (i >= inner.length) {
      return false;
    }
    if (inner.text[i] == 'E') {
      break;
    }
    size_t length = extract_ident_length(inner, &i);
    if (length == 0) {
      return false;
    }
    if (element_i > 0) {
      WRITE("::");
    }
    Name ident = {.text = &inner.text[i], .length = length};
    i += length;
    if (starts_with(ident, "_$")) {
      ident = take(ident, 1);
    }
    rust_demangle_ident(allocator, ident, output);
  }
  return true;
}

// V0 demangling is currently not implemented.
// RUST_FLAGS="-C symbol-mangling-version=legacy" can be used to force legacy
// mangling.
bool rust_demangle(Allocator *allocator, Name mangled, Name *output) {
  return rust_demangle_legacy(allocator, mangled, output);
}

Name demangle(Allocator *allocator, Name mangled) {
  Name output = {0};
  if (rust_demangle(allocator, mangled, &output)) {
    return output;
  }
  return mangled;
}
