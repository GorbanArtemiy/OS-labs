#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>

typedef int (*gcf_func)(int, int);
typedef float (*pi_func)(float);

int main() {
  void *gcf_lib = NULL;
  void *pi_lib = NULL;

  gcf_func gcf = NULL;
  pi_func pi = NULL;

  int use_euclid = 1;
  int use_leibniz = 1;

  char command[256];

  printf("\n=================================\n\n");
  printf("Commands:\n");
  printf("0: Switch implementations\n");
  printf("1 <int> <int>: Calculate GCF\n");
  printf("2 <float>: Calculate Pi\n");
  printf("3: Show current implementations\n");
  printf("4: Exit\n\n");
  printf("=================================\n\n");

  while (1) {
    printf("Enter command: ");
    fgets(command, sizeof(command), stdin);

    if (command[0] == '0') {
      use_euclid = !use_euclid;
      use_leibniz = !use_leibniz;
      printf("Switched implementations.\n\n");

    } else if (command[0] == '1') {
      int a, b;
      if (sscanf(command, "1 %d %d", &a, &b) != 2) {
        printf("Invalid input. Format: 1 <int> <int>\n\n");
        continue;
      }

      if (use_euclid) {
        if (!gcf_lib) {
          gcf_lib = dlopen("./libgcfeuclid.so", RTLD_LAZY);
        }
        if (!gcf) {
          gcf = dlsym(gcf_lib, "gcf_euclid");
        }
      } else {
        if (!gcf_lib) {
          gcf_lib = dlopen("./libgcfnaive.so", RTLD_LAZY);
        }
        if (!gcf) {
          gcf = dlsym(gcf_lib, "gcf_naive");
        }
      }

      if (!gcf_lib) {
        fprintf(stderr, "Error loading GCF library: %s\n", dlerror());
        return 1;
      }

      if (!gcf) {
        fprintf(stderr, "Error loading GCF symbol: %s\n", dlerror());
        return 1;
      }

      printf("GCF: %d\n\n", gcf(a, b));

    } else if (command[0] == '2') {
      float f;
      if (sscanf(command, "2 %f", &f) != 1) {
        printf("Invalid input. Format: 2 <float>\n\n");
        continue;
      }

      if (use_leibniz) {
        if (!pi_lib) {
          pi_lib = dlopen("./libpileibniz.so", RTLD_LAZY);
        }
        if (!pi) {
          pi = dlsym(pi_lib, "pi_leibniz");
        }
      } else {
        if (!pi_lib) {
          pi_lib = dlopen("./libpiwallace.so", RTLD_LAZY);
        }
        if (!pi) {
          pi = dlsym(pi_lib, "pi_wallace");
        }
      }

      if (!pi_lib) {
        fprintf(stderr, "Error loading Pi library: %s\n", dlerror());
        return 1;
      }

      if (!pi) {
        fprintf(stderr, "Error loading Pi symbol: %s\n", dlerror());
        return 1;
      }

      printf("Pi: %f\n\n", pi(f));

    } else if (command[0] == '3') {
      printf("Current implementations:\n");
      printf("GCF: %s\n",
             use_euclid ? "Euclid's algorithm" : "Naive algorithm");
      printf("Square: %s\n\n", use_leibniz ? "Leibniz" : "Wallace");

    } else if (command[0] == '4') {
      break;

    } else {
      printf("Unknown command. Valid commands:\n");
      printf("0: Switch implementations\n");
      printf("1 <int> <int>: Calculate GCF\n");
      printf("2 <float>: Calculate Pi\n");
      printf("3: Show current implementations\n");
      printf("4: Exit\n\n");
    }
  }

  if (gcf_lib) {
    dlclose(gcf_lib);
  }
  if (pi_lib) {
    dlclose(pi_lib);
  }

  return 0;
}
