## FibHeap

This is my personal implementation of a Fibonacci Heap and all supporting data structures needed
to implement Dijkstra's algorithm with it.

Most of the code conforms to the C99 standard, except for `main.c`. `main.c` utilizes a single GNU C extension: the `__transparent_union__` attribute for unions, as well as POSIX extensions (specifically for `strdup` and `getline`).