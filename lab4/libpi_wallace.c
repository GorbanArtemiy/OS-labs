#include "libpi.h"

float Pi_wallace(int K) {
    float pi = 2.0, term;
    for (int n = 1; n < K; n++) {
        term = (4.0f * n * n) / (4.0f * n * n - 1.0f);
        pi *= term;
    }
    return pi;
}
