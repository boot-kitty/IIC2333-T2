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
  printf("Slots de procesos disponibles: %d\n", processes_slots());

  printf("\nArchivos del proceso con ID 198:\n");
  list_files(198);

  printf("\nEstado del bitmap de frames:\n");
  frame_bitmap_status();

  // if (strcmp(memory_path, "memorias/copyfill.bin") == 0) {
  //   printf("\nFormateando memoria...\n");
  //   format_memory(memory_path);
  //   mount_memory(memory_path);
  //   printf("Procesos en ejecución:\n");
  //   list_processes();
  //   printf("Slots de procesos: %d\n", processes_slots());

  //   printf("\nArchivos del proceso con ID 198:\n");
  //   list_files(198);

  //   printf("\nEstado del bitmap de frames:\n");
  //   frame_bitmap_status();
  // }

  return 0;
}