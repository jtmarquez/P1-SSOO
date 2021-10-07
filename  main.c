// Import used global libraries
#include <stdio.h>  // FILE, fopen, fclose, etc.
#include <string.h> // strtok, strcpy, etc.
#include <stdlib.h> // malloc, calloc, free, etc.

InputFile* read_file(char* filename)
{
  // Read the file
  FILE* file_pointer = fopen(filename, "r");

  // Declare a buffer and read the first line
  char buffer[BUFFER_SIZE];
  fgets(buffer, BUFFER_SIZE, file_pointer);

  // Define the struct and prepare its attributes
  InputFile* input_file = malloc(sizeof(InputFile));
  input_file->lines = calloc(atoi(buffer), sizeof(char**));
  input_file->len = atoi(buffer);

  // Parse each line
  int index = 0;
  while (fgets(buffer, BUFFER_SIZE, file_pointer)) {
    input_file->lines[index++] = split_by_sep(buffer, " ");
  }

  // Close the file and return the struct
  fclose(file_pointer);
  return input_file;
}

/*
* Frees all the memory used by "input_file".
*/
void input_file_destroy(InputFile* input_file)
{
  // Free each separated data line and its contents
  for (int i = 0; i < input_file->len; i++) {
    for (int j = 0; j < MAX_SPLIT; j++) {
      free(input_file->lines[i][j]);
    }
    free(input_file->lines[i]);
  }

  // Free the lines array
  free(input_file->lines);

  // Free the input_file itself
  free(input_file);
}

int main(int argc, char **argv)
{
  printf("Hello T2!\n");
  char *input_name;
  char *output_name;
  input_name = argv[1];
  char *filename = input_name;
 
  InputFile *file = read_file(filename);
  
  struct proceso* array_procesos= calloc(file->len, sizeof(struct proceso));;
  
  printf("Reading file of length %i:\n", file->len);
  printf("\n");
}