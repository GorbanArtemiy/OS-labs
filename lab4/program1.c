#include "libgcf.h"
#include "libpi.h"
#include <stdio.h>

int main() {
  int a, b;
  float f;

  printf("Enter two integers for GCF: ");
  scanf("%d %d", &a, &b);
  printf("GCF (Euclid): %d\n", gcf_euclid(a, b));

  printf("Enter one float for Pi: ");
  scanf("%f", &f);
  printf("Pi: %f\n", Pi_leibniz(f));

  return 0;
}
