#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NAME 100

int main() {
  char *name = (char *) malloc(NAME);

  if (name == NULL)
    return -1;

  free(name);
  free(name);
  return 0;
}
