# Cera

Cera is a general purpose programming language. This repository holds its compiler.

## Dependencies

GNU C11
LLVM

### Debugging

Sanitizers: `libasan`, `libubsan`

`addr2line` used on Linux to print backtraces.

## Semantics

`int` and `uint` are pointer-sized.

`.ce` is the file extension for Cera files.
