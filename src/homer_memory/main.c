#include <stdio.h>
#include "../homer_memory_API/homer_memory_API.h"

int main(int argc, char const *argv[]) {
  // montar la memoria
  char* memory_path = "memorias/memfilled.bin";
  if (argc > 1) {
    memory_path = (char *)argv[1];
  }
  mount_memory(memory_path);

  printf("### Procesos en ejecución:\n");
  list_processes();
  printf("\n### Slots de procesos disponibles: %d\n", processes_slots());
  printf("\n### Estado del bitmap de frames:\n");
  frame_bitmap_status();
  printf("\n### Archivos del proceso con ID 198:\n");
  list_files(198);

  start_process(10, "Proceso10");
  start_process(20, "Proceso20");
  printf("\n### Slots de procesos disponibles después de iniciar 2 procesos: %d\n", processes_slots());

  printf("\n### Procesos en ejecución:\n");
  list_processes();

  printf("\n### Archivos del proceso con ID 10:\n");
  list_files(10);

  finish_process(198);
  printf("\n### Procesos en ejecución después de finalizar proceso 198:\n");
  list_processes();
  printf("\n### Slots de procesos disponibles después de finalizar proceso 198: %d\n", processes_slots());
  printf("\n### Estado del bitmap de frames después de finalizar proceso 198:\n");
  frame_bitmap_status();

  printf("\n### Limpiando todos los procesos...\n");
  clear_all_processes();
  printf("\n### Procesos en ejecución después de limpiar:\n");
  list_processes();
  printf("\n### Slots de procesos disponibles después de limpiar: %d\n", processes_slots());
  printf("\n### Estado del bitmap de frames después de limpiar:\n");
  frame_bitmap_status();

  printf("\nDeseas formatear la memoria? (y/n): ");
  char response;
  scanf(" %c", &response);
  if (response == 'y' || response == 'Y') {
    printf("\nFormateando memoria...\n");
    format_memory(memory_path);
    mount_memory(memory_path);
    printf("Procesos en ejecución:\n");
    list_processes();
    printf("Slots de procesos: %d\n", processes_slots());

    printf("\nArchivos del proceso con ID 198:\n");
    list_files(198);

    printf("\nEstado del bitmap de frames:\n");
    frame_bitmap_status();
  } else {
    printf("\nMemoria no formateada. Saliendo...\n");
  }

  return 0;
}