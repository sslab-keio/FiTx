#include <stdio.h>
#include <stdlib.h>

struct test {
  char* name;
  char* value;
};


int alloc_member(struct test* t) {
  t->name = (char*)malloc(10);

  if (t->name)
    return - 15;

  return 0;
}

void another_free(struct test* t) {
  free(t);
}

int some_alloc(struct test* t) {
  int err = 0;
  t = (struct test*)malloc(sizeof(struct test));
  if (!t)
    return -10;

  err = alloc_member(t);
  if (err)
    goto out;

  return 0;
out:
  another_free(t);
  return err;
}

int main() {
  struct test* t;
  int err = some_alloc(t);
  if (err) {
    another_free(t);
    printf("ERROR");
    return err;
  }

  return 0;
}
