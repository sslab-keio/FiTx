#include <stdio.h>
#include <stdlib.h>

struct test {
  int id;
  char* name;
};

int allocate_something(struct test* test) {
  if (!(test->name = malloc(100)))
    return -1;
  return 0;
}

int main() {
  int *some_value = (int*)malloc(10);
  return 0; // BUG: memory leak of |some_value| here
}
