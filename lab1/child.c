#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

int main() {
  char str[BUFFER_SIZE];
  size_t str_sz;
  memset(str, 0, BUFFER_SIZE);

  if (read(STDIN_FILENO, &str_sz, sizeof(size_t)) < 0 ||
      read(STDIN_FILENO, str, sizeof(char) * str_sz) < 0) {
    perror("Reading failed");
    exit(-1);
  }

  FILE *output = fopen(str, "w");
  int fd = fileno(output);
  dup2(fd, STDOUT_FILENO);

  if (output == NULL) {
    perror("The file could not be opened");
    exit(-1);
  }

  while (read(STDIN_FILENO, &str_sz, sizeof(size_t)) > 0) {
    memset(str, 0, BUFFER_SIZE);

    if (read(STDIN_FILENO, str, sizeof(char) * str_sz) < 0) {
      perror("Reading failed");
      exit(-1);
    }

    if (isupper(str[0])) {
      printf("%s\n", str);
    } else {
      fprintf(stderr, "Error: line '%s' is invalid\n", str);
    }
  }

  fclose(output);
  return 0;
}
