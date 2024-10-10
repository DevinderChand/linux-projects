#include <windows.h>
#include <string.h>
// Remove or comment out the following lines:
// #include <sys/ipc.h>
// #include <sys/shm.h>
// #include <sys/wait.h>

#define SHM_SIZE 1024 // Size of shared memory

int main() {
  key_t key;
  int shmid;
  char *shared_memory;

  // Step 1: Generate a unique key for shared memory using /tmp as a base file
  key = ftok("/tmp", 65);
  if (key == -1) {
    perror("ftok");
    exit(1);
  }

  // Step 2: Create a shared memory segment
  shmid = shmget(key, SHM_SIZE, 0666 | IPC_CREAT);
  if (shmid == -1) {
    perror("shmget");
    exit(1); 
  }

  // Step 3: Fork a new process
  pid_t pid = fork();

  if (pid < 0) {
    perror("fork");
    exit(1);
  }

  if (pid == 0) { // Child process
    // Step 4: Attach to the shared memory in the child process
    shared_memory = (char *)shmat(shmid, NULL, 0);
    if (shared_memory == (char *)-1) {
      perror("shmat");
      exit(1);
    }

    // Step 5: Write data to the shared memory
    char message[] = "Hello from child process!";
    printf("Child: Writing to shared memory: %s\n", message);
    strncpy(shared_memory, message, SHM_SIZE);

    // Detach from shared memory
    shmdt(shared_memory);
    exit(0);
  } else { // Parent process
    // Wait for child process to complete
    wait(NULL);

    // Step 6: Attach to the shared memory in the parent process
    shared_memory = (char *)shmat(shmid, NULL, 0);
    if (shared_memory == (char *)-1) {
      perror("shmat");
      exit(1);
    }

    // Step 7: Read the data from the shared memory
    printf("Parent: Reading from shared memory: %s\n", shared_memory);

    // Detach from shared memory
    shmdt(shared_memory);

    // Step 8: Destroy the shared memory segment
    shmctl(shmid, IPC_RMID, NULL);
  }

  return 0;
}
