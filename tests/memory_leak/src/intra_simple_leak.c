#include <stdio.h>
#include <stdlib.h>

int main() {
  int *some_value = (int*)malloc(10); // BUG: Memory leak of `some_value` here
  return 0;
}
