#include <stdio.h>
#include <stdlib.h>

struct spin_lock {
  int id;
  int val;
};

int spin_lock(struct spin_lock* lock){ return 0; };
int spin_unlock(struct spin_lock* lock){return 0; };

int main() {
  struct spin_lock* lock = (struct spin_lock*)malloc(sizeof(struct spin_lock));

  spin_lock(lock);
  spin_lock(lock);

  return 0;
}
