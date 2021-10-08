#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "../file_manager/manager.h"
#include <string.h>


int main(int argc, char **argv)
{
  printf("Hello P1!\n");
  char *input_name;
  input_name = argv[1];
  char *filename = input_name;

  unsigned char buffer[4000];
  FILE *ptr;

  ptr = fopen(filename,"rb");  // r for read, b for binary

  fread(buffer,sizeof(buffer),1,ptr); // read 10 bytes to our buffer

  char string[12];
  char bytes[12];
  for(int i = 0; i<1024; i++){
   printf("%d",buffer[i]);
   if (i==255 || i==511 || i==767 || i==1023){
     printf("\ncambio\n");
   }
  }

}