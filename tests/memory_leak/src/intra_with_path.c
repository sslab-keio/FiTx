#include <stdio.h>
#include <stdlib.h>

struct test {
  int id;
  char* name;
};

int main() {
  int *some_value = (int*)malloc(10);
  int err;
  if (some_value != NULL) {
    printf("Some Value");
    goto err;
  }
  /* free(some_value); */
err:
  return 0;
}
