#include <stdio.h>
#include <stdlib.h>

#define on_error_exit(...) { fprintf(stderr, __VA_ARGS__); fflush(stderr); exit(1); }
