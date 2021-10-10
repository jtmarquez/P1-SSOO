#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "../file_manager/manager.h"
#include "crms_API.h"

#define N_ENTRADAS_PCB 16
#define TAMANO_ENTRADA_PCB 256
#define TAMANO_SUBENTRADA_PCB 21
#define TAMANO_SUBENTRADA_PCB_NOMBRE_ARCHIVO 12

char *filename;
FILE *memory_file;
unsigned char buffer[5000];

/*Funcion para montar la memoria.
  Establece como variable global la ruta local donde se encuentra el archivo .bin 
  correspondiente a la memoria.*/

 void cr_mount(char* memory_path)
  {
    memory_file = malloc(sizeof(FILE));
    memory_file[0] = *(fopen(memory_path,"rb+"));
    //memory_file = fopen(memory_path,"rb");
  }



/* Funcion que muestra en pantalla los procesos en ejecucion.*/
void cr_ls_processes()
  {fseek(memory_file, 0 ,SEEK_SET);
    fread(buffer,sizeof(buffer),1,memory_file); // read 10 bytes to our buffer*/
    int num = 1;
    int aux;
    for(int i = 0; i < N_ENTRADAS_PCB*TAMANO_ENTRADA_PCB; i += TAMANO_ENTRADA_PCB){
      if (i == 0 || !(i % TAMANO_ENTRADA_PCB)){
        if (buffer[i] == 1)
        {
          aux = i + 1;
          printf("\n [Entrada: %d, Proceso en ejecucion: %d] \n",num, buffer[aux] );
        }

      cont += sum;
      num +=1;
      }
    //printf("%d", buffer[i]);

    }
  }
/* Funcion para ver si un archivo con nombre file name existe en la memoria del proceso con id process id. 
Retorna 1 si existe y 0 en caso contrario.*/
int cr_exists(int process_id, char* file_name)
  {

    fseek(memory_file, 0 ,SEEK_SET);
    fread(buffer,sizeof(buffer),1,memory_file);
    int cont = 0;
    int sum = 256;

    int existe = 0;

    for(int i = 0; i < N_ENTRADAS_PCB*TAMANO_ENTRADA_PCB; i += TAMANO_ENTRADA_PCB)
    {
      if (i == 0 || !(i % TAMANO_ENTRADA_PCB)){ //si estoy al inicio de una de las entradas (i mod 256 = 0)
        if (buffer[i] == 1) //si el proceso esta en ejecucion (bit validez = 1)
        {
          if (buffer[i+1] == process_id) //si es el proceso que busco
          {
            //printf("encontre el  proceso %d = %d \n", buffer[i+1], process_id);
            int inicio = i + 14; //donde empiezan las subentradas de archivos
            for (int j = inicio; j <= (i + 14 + 210); j += TAMANO_SUBENTRADA_PCB) //10 entradas de 21 bits cada una
            {//printf("j es: %d y inicio es %d\n", j, inicio);
              if ((j - inicio) == 0 || !((j - inicio) % TAMANO_SUBENTRADA_PCB)) //si estoy al inicio de una subentrada (j - inicio mod 21 = 0)
                {
                  existe = 1;
                  if (buffer[j] == 1)//si la subentrada es valida
                  {
                    int cont_filename = 0;
                    for (int k = j; k < j + TAMANO_SUBENTRADA_PCB_NOMBRE_ARCHIVO; k++){
                      if (file_name[cont_filename] != buffer[k+1]){
                        existe = 0;
                      }
                      cont_filename += 1;
                    }
                    if (existe == 1){
                      return existe;
                    }
                  }   
                } 
            }
          } 
        }
      }
    }
    existe = 0;
    return existe;
  }

//Funcion para listar los archivos dentro de la memoria del proceso. 
//Imprime en pantalla los nombres de todos los archivos presentes en 
//la memoria del proceso con id process id.


void cr_ls_files(int process_id){
    fseek(memory_file, 0 ,SEEK_SET);
    fread(buffer,sizeof(buffer),1,memory_file);
    int cont = 0;
    int sum = 256;

    for(int i = 0; i<N_ENTRADAS_PCB*TAMANO_ENTRADA_PCB; i++)
    {
      if (i == cont){ //si estoy al inicio de una de las entradas
        if (buffer[i] == 1) //si el proceso esta en ejecucion (bit validez = 1)

        {
          //printf("encontre el  proceso %d = %d \n", buffer[i+1], process_id);
          int inicio = i + 14; //donde empiezan las subentradas de archivos
          for (int j = inicio; j <= (i + 14 + 210); j += TAMANO_SUBENTRADA_PCB) //10 entradas de 21 bits cada una
          {//printf("j es: %d y inicio es %d\n", j, inicio);
            if ((j - inicio) == 0 || !((j - inicio) % TAMANO_SUBENTRADA_PCB)) //si estoy al inicio de una subentrada
              {
                //printf("ENTRADA\n");
                if (buffer[j] == 1)//si la subentrada es valida
                {
                  for (int k = j; k<= j+ 12; k++){printf("%c", buffer[k]);}
                  printf("\n");
                } 
              }
          }
        }
      }
    }
  }
}

 //Funcion que inicia un proceso con id process id y nombre process name. 
 //Guarda toda la informacion correspondiente en una entrada en la tabla de PCBs.
void cr_start_process(int process_id, char* process_name)
{
  fseek(memory_file, 0 ,SEEK_SET);
  fread(buffer,sizeof(buffer),1,memory_file);
  int cont = 0;
  int sum = 256;

  for(int i = 0; i < N_ENTRADAS_PCB*TAMANO_ENTRADA_PCB; i++)
  {
    if (i == cont){ //si estoy al inicio de una de las entradas
      if (buffer[i] == 0) //si el proceso esta en ejecucion (bit validez = 1)
      {
        printf("encontre la entrada %d vacia\n", cont/256);
        char validation[4];
        int uno = 1;
        sprintf(validation, "%d", uno);
        printf("%s \n", validation);
        char validation_char = (char) validation;

        fseek(memory_file, i*sizeof(char),SEEK_SET);
        fwrite(&validation_char, sizeof(char), 1, memory_file);

        char pid[4];
        sprintf(pid, "%d", process_id);
        char pid_char = (char) pid;
        printf("%s \n", pid);
        fseek(memory_file, (i+1)*sizeof(char),SEEK_SET);
        fwrite(&pid_char, sizeof(char), 1, memory_file);
        for (int j = 0; j < strlen(process_name); j++)
        {
          char letter = (char)process_name[j];
          printf("%c \n", letter);
          fseek(memory_file, (i+2+j)*sizeof(char),SEEK_SET);
          fwrite(&letter, sizeof(char), 1, memory_file);
        }
        continue;
      }
    cont += sum;
    }
  }
  fclose(memory_file);
}

void cr_finish_process(int process_id) 
{
  int cont = 0;
  int sum = 256;

  for(int i = 0; i < N_ENTRADAS_PCB*TAMANO_ENTRADA_PCB; i++)
  {
    if (i == cont){ //si estoy al inicio de una de las entradas
      if (buffer[i] == 0) //si el proceso esta en ejecucion (bit validez = 1)
      {
        if (buffer[i+1] == process_id) //si encuentro el proceso correspondiente
        {
          buffer[i+1] = 0; //invalido el proceso correspondiente
          //obtengo al direccion virtual correspondiente al proceso
        }
      }
    cont += sum;
    }
  }
  //obtengo los primeros 5 bits = VPN
  // ingreso a esa entrada de la tabla de paginas
  //obtengo el PFN
  // direccion fisica = PFN + offset
}

void print_memory(char* filename){
  cr_mount(filename);
  fseek(memory_file, 0 ,SEEK_SET);
  fread(buffer,sizeof(buffer),1,memory_file); // read 10 bytes to our buffer*/
  int cont = 0;
  int sum = 256;
  int num = 1;

  for(int i = 0; i < N_ENTRADAS_PCB*TAMANO_ENTRADA_PCB; i++){
    if (i == cont){ 
        printf("\n [Entrada: %d] \n",num);
    cont += sum;
    num +=1;
    }
    printf("%d", buffer[i]);
  }
}

int main(int argc, char **argv)
{
  printf("Hello P1!\n");
  char *input_name;
  input_name = argv[1];
  filename = input_name;
  printf("\n");
  printf("-------Ejecutando la funcion cr_mount-------\n");
  printf("\n");
  cr_mount(filename);
  printf("\n");
  printf("-------Ejecutando la funcion cr_ls_processes--------\n");
  printf("\n");
  cr_ls_processes();
  printf("\n");
  printf("-------Ejecutando la funcion cr_exists-----------\n");
  printf("\n");

  cr_exists(200, "gato.mp4");
  printf("\n");
  print_memory(filename);
  printf("-------Ejecutando la funcion cr_start-----------\n");
  printf("\n");
  cr_start_process(3, "test1");
  printf("\n");
  print_memory(filename);

}