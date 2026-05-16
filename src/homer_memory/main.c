#include <stdio.h>
#include "../homer_memory_API/homer_memory_API.h"

int main(int argc, char const *argv[]) {
  // montar la memoria
  char* memory_path = "memorias/memfilled.bin";
  if (argc > 1) {
    memory_path = (char *)argv[1];
  }
  mount_memory(memory_path);

  printf("Procesos en ejecución:\n");
  list_processes();
  printf("Slots de procesos: %d\n", processes_slots());

  list_files(198);

  return 0;
}