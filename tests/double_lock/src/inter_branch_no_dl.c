#include <stdio.h>
#include <stdlib.h>

struct spin_lock {
  int id;
  int val;
};

struct data {
  struct spin_lock lock;
  int data;
};

int spin_lock(struct spin_lock* lock){ return 0; };
int spin_unlock(struct spin_lock* lock){return 0; };

int check(struct data* dat) {
  int ret = 0;

  spin_lock(&dat->lock);
  if (dat->data != 1) {
    ret = 1;
  }
  spin_unlock(&dat->lock);

  return ret;
}

int main() {
  struct data* dat = (struct data*)malloc(sizeof(struct data));
  if (check(dat))
    spin_lock(&dat->lock);

  return 0;
}
