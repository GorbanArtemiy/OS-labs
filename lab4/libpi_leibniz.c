#include "libpi.h"

float Pi_leibniz(int K) {
    float pi = 0.0;
    int sign = 1;
    for (int n = 0; n < K; n++) {
        pi += sign * 4.0f / (2 * n + 1);
        sign *= -1;
    }
    return pi;
}
