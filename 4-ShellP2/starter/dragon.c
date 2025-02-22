#include <stdio.h>
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
