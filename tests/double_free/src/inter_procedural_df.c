#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NAME 100

void free_char(char* name) {
  free(name);
}

int main() {
  char *name = (char *) malloc(NAME);

  if (name == NULL)
    return -1;

  free_char(name);
  free(name);
  return 0;
}
