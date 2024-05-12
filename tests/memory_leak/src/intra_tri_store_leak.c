#include <stdio.h>
#include <stdlib.h>

struct tmp {
  char* name;
};

char* allocate(struct tmp* tmp) {
  char* name;
  name = tmp->name = (char*)malloc(100);
  return name;
}

int main() {
  struct tmp tmp;
  allocate(&tmp);
  return 0; // BUG: Memory leak of tmp->name here
}
