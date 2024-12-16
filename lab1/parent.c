#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

int CreateProcess() {
  pid_t pid = fork();
  if (pid == -1) {
    perror("Fork failed");
    exit(-1);
  }
  return pid;
}

int main() {
  int pipe1[2], pipe2[2];

  if (pipe(pipe1) == -1 || pipe(pipe2) == -1) {
    perror("Pipe creation failed");
    exit(-1);
  }

  pid_t pid = CreateProcess();

  if (pid == 0) {
    close(pipe1[1]);
    close(pipe2[0]);

    dup2(pipe1[0], STDIN_FILENO);
    dup2(pipe2[1], STDERR_FILENO);

    execl("./child", "child", NULL);

    perror("Execl failed");
    exit(-1);

  } else {
    close(pipe1[0]);
    close(pipe2[1]);

    char str[BUFFER_SIZE];
    size_t str_sz;

    printf("Enter the filename for the output: ");
    fgets(str, BUFFER_SIZE, stdin);
    str[strlen(str) - 1] = '\0';
    str_sz = strlen(str);

    if (write(pipe1[1], &str_sz, sizeof(size_t)) < 0 ||
        write(pipe1[1], str, sizeof(char) * str_sz) < 0) {
      perror("Writing failed");
      exit(-1);
    }

    printf("Enter text (press Ctrl+D to send EOF):\n");
    while (fgets(str, BUFFER_SIZE, stdin) != NULL) {
      if (strlen(str) == 1 && str[0] == '\n') {
        break;
      }

      str[strlen(str) - 1] = '\0';
      str_sz = strlen(str);

      if (write(pipe1[1], &str_sz, sizeof(size_t)) < 0 ||
          write(pipe1[1], str, sizeof(char) * str_sz) < 0) {
        perror("Writing failed");
        exit(-1);
      }
    }

    close(pipe1[1]);

    wait(NULL);

    size_t bytes_read;
    while ((bytes_read = read(pipe2[0], str, BUFFER_SIZE)) > 0) {
      printf(str, bytes_read);
    }

    close(pipe2[0]);
  }

  return 0;
}
