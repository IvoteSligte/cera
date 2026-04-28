
main :: () {
  doStuff(10, 20);
}

doStuff :: (a: int, b: int) {
  for i := a; i < b; i = i + 1; {
    print_string("Hello, \"World\"!\n");
  }
}

