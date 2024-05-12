#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NAME 100

struct test {
  int id;
  char* name;
};

int main() {
  struct test* test = (struct test*)malloc(sizeof(struct test));
  if (test == NULL)
    return -1;

  test->name = (char *) malloc(NAME);
  if (test->name == NULL)
    return -1;

  free(test->name);
  free(test);
  return 0;
}
