#include "libgcf.h"

int gcf_naive(int a, int b) {
  int result = 1;
  for (int i = 1; i <= a && i <= b; i++) {
    if (a % i == 0 && b % i == 0) {
      result = i;
    }
  }
  return result;
}
