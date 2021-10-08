#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "../file_manager/manager.h"
#include "crms_API.h"

#define N_ENTRADAS_PCB 16
#define TAMANO_ENTRADA_PCB 256

char *filename;
FILE *memory_file;
unsigned char buffer[5000];

/*Funcion para montar la memoria.
  Establece como variable global la ruta local donde se encuentra el archivo .bin 
  correspondiente a la memoria.*/
 void cr_mount(char* memory_path)
  {
    memory_file = fopen(memory_path,"rb");
  }


/* Funcion que muestra en pantalla los procesos en ejecucion.*/
void cr_ls_processes()
  {
    fread(buffer,sizeof(buffer),1,memory_file); // read 10 bytes to our buffer*/
    int cont = 0;
    int sum = 256;
    int num = 1;
    int aux;
    for(int i = 0; i < N_ENTRADAS_PCB*TAMANO_ENTRADA_PCB; i++){
      if (i == cont){
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
    int cont = 0;
    int sum = 256;
    int existe = 0;

    for(int i = 0; i < N_ENTRADAS_PCB*TAMANO_ENTRADA_PCB; i++)
    {
      if (i == cont){ //si estoy al inicio de una de las entradas
        if (buffer[i] == 1) //si el proceso esta en ejecucion (bit validez = 1)
        {
          if (buffer[i+1] == process_id) //si es el proceso que busco
          {
            //printf("encontre el  proceso %d = %d \n", buffer[i+1], process_id);
            int inicio = i + 14; //donde empiezan las subentradas de archivos
            int suma = 21;
            for (int j=inicio; j <= (i + 14 + 210); j++) //10 entradas de 21 bits cada una
            {//printf("j es: %d y inicio es %d\n", j, inicio);
              if (j==inicio) //si estoy al inicio de una subentrada
                {
                  existe = 1;
                  if (buffer[j] == 1)//si la subentrada es valida
                  {
                  int cont_filename = 0;
                  for (int k = j; k< j+ 12; k++){
                    if (file_name[cont_filename]!= buffer[k+1]){existe = 0;}
                    cont_filename += 1;
                    } if (existe == 1){return existe; }
                  }   
              inicio += suma;
                } 
            }
          } 
        }
    cont += sum;
    }
  }
  existe = 0;
  return existe;}

//Funcion para listar los archivos dentro de la memoria del proceso. 
//Imprime en pantalla los nombres de todos los archivos presentes en 
//la memoria del proceso con id process id.
void cr_ls_files(int process_id)
{
int cont = 0;
    int sum = 256;

    for(int i = 0; i<N_ENTRADAS_PCB*TAMANO_ENTRADA_PCB; i++)
    {
      if (i == cont){ //si estoy al inicio de una de las entradas
        if (buffer[i] == 1) //si el proceso esta en ejecucion (bit validez = 1)
        {
          if (buffer[i+1] == process_id) //si es el proceso que busco
          {
            //printf("encontre el  proceso %d = %d \n", buffer[i+1], process_id);
            int inicio = i + 14; //donde empiezan las subentradas de archivos
            int suma = 21;
            for (int j=inicio; j<= (i + 14 + 210); j++) //10 entradas de 21 bits cada una
            {//printf("j es: %d y inicio es %d\n", j, inicio);
              if (j==inicio) //si estoy al inicio de una subentrada
                {
                  //printf("ENTRADA\n");
                  if (buffer[j] == 1)//si la subentrada es valida
                  {
                   for (int k = j; k<= j+ 12; k++){printf("%c", buffer[k]);}
                   printf("\n");
                  }
                  
              inicio += suma;
                }
              
             // printf("%d", buffer[j]);
            }
          
          }
      
        }
    cont += sum;
    }
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
  int existe = cr_exists(200, "hecomes.mp4");
  if (existe == 1){printf("El archivo SI esta almacenado en el proceso\n ");}
  else {printf("El archivo NO esta almacenado en el proceso\n ");}
  printf("\n");
  printf("-------Ejecutando la funcion cr__ls_files-----------\n");
  printf("\n");
  cr_ls_files(200);

}