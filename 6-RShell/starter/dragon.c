#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include "dshlib.h"

#define DRAGON_FILE "dragon.txt"

extern void print_dragon(){
  FILE *file = fopen(DRAGON_FILE, "r");
  if (file == NULL)
  {
      printf("Could not find the dragon file\n");
  }
  char line[512];
  while (fgets(line, sizeof(line), file))
  {
      printf("%s", line);
  }

  fclose(file);
  printf("\n");
}

extern int print_dragon_to_socket(int cli_socket){
  FILE *file = fopen(DRAGON_FILE, "r");
  if (file == NULL)
  {
    perror("fopen");
    return ERR_EXEC_CMD;
  }
  char line[1024];
  while (fgets(line, sizeof(line), file) != NULL) {
      send(cli_socket, line, strlen(line), 0);
  }
  send(cli_socket, "\n", 1, 0);
  fclose(file);
  return OK;
}
