# FibHeap

This is my personal implementation of a Fibonacci Heap and all supporting data structures needed
to implement Dijkstra's algorithm with it.

## Compilation

Most of the code conforms to the C99 standard, except for `main.c`. `main.c` utilizes POSIX extensions (specifically for `strdup` and `getline`) and a single GNU C extension: the `__transparent_union__` attribute for unions.