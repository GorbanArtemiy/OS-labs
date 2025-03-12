#include "libgcf.h"

int gcf_euclid(int a, int b) {
  while (b) {
    a %= b;
    int temp = a;
    a = b;
    b = temp;
  }
  return a;
}
