#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NAME 100

struct test {
  int id;
  char* name;
};

void free_char(struct test* t) {
  free(t->name);
  t->name = NULL;
}

int main() {
  struct test* name = (struct test*) malloc(sizeof(struct test));

  if (name == NULL)
    return -1;

  free_char(name);

  free(name->name);
  free(name);
  return 0;
}
