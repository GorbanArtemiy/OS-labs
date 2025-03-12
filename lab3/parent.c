#include <fcntl.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#define BUFFER_SIZE 1024
#define SHARED_MEM_SIZE 4096
#define SEM_NAME "/my_semaphore"

struct shared_data {
  char filename[BUFFER_SIZE];
  char line[BUFFER_SIZE];
  char result[BUFFER_SIZE];
  int parent_turn;
};

int main() {
  int fd_shm;
  struct shared_data *shared_memory;
  sem_t *semaphore;
  pid_t pid;

  fd_shm = shm_open("/my_shared_memory", O_CREAT | O_RDWR, 0666);
  if (fd_shm == -1) {
    perror("shm_open");
    exit(1);
  }
  ftruncate(fd_shm, sizeof(struct shared_data));
  shared_memory = mmap(NULL, sizeof(struct shared_data), PROT_READ | PROT_WRITE,
                       MAP_SHARED, fd_shm, 0);
  if (shared_memory == MAP_FAILED) {
    perror("mmap");
    exit(1);
  }

  semaphore = sem_open(SEM_NAME, O_CREAT, 0666, 0);
  if (semaphore == SEM_FAILED) {
    perror("sem_open");
    exit(1);
  }

  pid = fork();

  if (pid == 0) {
    execl("./child", "child", NULL);
    perror("execl");
    exit(1);

  } else if (pid > 0) {
    printf("Enter the filename for the output: ");
    fgets(shared_memory->filename, BUFFER_SIZE, stdin);
    shared_memory->filename[strcspn(shared_memory->filename, "\n")] = 0;

    printf("Enter text (press Ctrl+D to send EOF):\n");
    while (fgets(shared_memory->line, BUFFER_SIZE, stdin) != NULL) {
      shared_memory->line[strcspn(shared_memory->line, "\n")] = 0;
      shared_memory->parent_turn = 0;
      sem_post(semaphore);
      while (shared_memory->parent_turn == 0) {
      };
      fprintf(stderr, "%s", shared_memory->result);
    }

    shared_memory->parent_turn = -1;
    sem_post(semaphore);
    wait(NULL);

    munmap(shared_memory, sizeof(struct shared_data));
    shm_unlink("/my_shared_memory");
    sem_close(semaphore);
    sem_unlink(SEM_NAME);

  } else {
    perror("fork");
    exit(1);
  }
}
