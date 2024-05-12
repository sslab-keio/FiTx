#include <stdio.h>
#include <stdlib.h>

void alloc_again(int** val) {
  *val = (int*)malloc(10);
}

int main() {
  int *some_value = (int*)malloc(10);

  free(some_value);
  alloc_again(&some_value);

  int x = *some_value + 10;
  printf("%d", x);
  return 0;
}
