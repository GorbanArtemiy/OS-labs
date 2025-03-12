#include <fcntl.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
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

  fd_shm = shm_open("/my_shared_memory", O_RDWR, 0666);
  if (fd_shm == -1) {
    perror("shm_open");
    exit(1);
  }

  shared_memory = mmap(NULL, sizeof(struct shared_data), PROT_READ | PROT_WRITE,
                       MAP_SHARED, fd_shm, 0);
  if (shared_memory == MAP_FAILED) {
    perror("mmap");
    exit(1);
  }

  semaphore = sem_open(SEM_NAME, 0);
  if (semaphore == SEM_FAILED) {
    perror("sem_open");
    exit(1);
  }

  while (1) {
    sem_wait(semaphore);
    if (shared_memory->parent_turn == -1) {
      break;
    }

    FILE *output = fopen(shared_memory->filename, "a");

    if (output == NULL) {
      snprintf(shared_memory->result, BUFFER_SIZE,
               "Error: could not open file %.255s", shared_memory->filename);
    } else {

      if if (isupper(shared_memory->line[0])) {
        fprintf(output, "%s\n", shared_memory->line);
        shared_memory->result[0] = '\0';
      } else {
        snprintf(shared_memory->result, BUFFER_SIZE,
                 "Error: line '%.255s' is invalid\n", shared_memory->line);
      }

      fclose(output);
    }
    shared_memory->parent_turn = 1;
  }

  munmap(shared_memory, sizeof(struct shared_data));
  sem_close(semaphore);
}
