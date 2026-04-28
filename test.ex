
main :: () {
  doStuff(10, 20);
}

doStuff :: (a: int, b: int) {
  for (i: int = a; i < b; i = i + 1) {
    print_string("Hello, World!");
  }
}

