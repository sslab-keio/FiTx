#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NAME 100

void show_name(char* name) {
  printf("%s\n", name);
}

int if_name_free(char* name) {
  /* if(strcmp(name, "free") == 0) */
  /*   return 0; */
  return 1;
}

void free_char(char* name) {
  free(name);
}

int main() {
  char *name = (char *) malloc(NAME);

  if (name == NULL)
    return -1;

  if (if_name_free(name) != 0) {
    free(name);
  }

  /* show_name(name); */
  free(name);
  return 0;
}
