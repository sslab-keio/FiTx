#include <stdio.h>
#include <stdlib.h>

int main() {
  int *some_value = (int*)malloc(10);

  free(some_value);
  int x = *some_value + 10;
  printf("%d", x);
  return 0;
}
