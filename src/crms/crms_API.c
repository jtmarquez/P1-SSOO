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
#define TAMANO_SUBENTRADA_PCB_TAMANO_ARCHIVO 4
#define TAMANO_SUBENTRADA_PCB_DIRECCION_VIRTUAL 4 
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
    int num = 1;
    int aux;
    for(int i = 0; i < N_ENTRADAS_PCB*TAMANO_ENTRADA_PCB; i += TAMANO_ENTRADA_PCB){
      if (i == 0 || !(i % TAMANO_ENTRADA_PCB)){
        if (buffer[i] == 1)
        {
          aux = i + 1;
          printf("\n [Entrada: %d, Proceso en ejecucion: %d] \n",num, buffer[aux] );
        }
        num +=1;
      }
      //printf("%d", buffer[i]);
    }
  }
/* Funcion para ver si un archivo con nombre file name existe en la memoria del proceso con id process id. 
Retorna 1 si existe y 0 en caso contrario.*/
int cr_exists(int process_id, char* file_name)
  {
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
void cr_ls_files(int process_id)
{
  int limit, base;
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
            if ((j - inicio) == 0 || !((j - inicio) % TAMANO_SUBENTRADA_PCB)) //si estoy al inicio de una subentrada
              {
                //printf("ENTRADA\n");
                if (buffer[j] == 1)//si la subentrada es valida
                {
                  printf("%d\n", buffer[j]);
                  for (int k = j; k <= j + TAMANO_SUBENTRADA_PCB_NOMBRE_ARCHIVO; k++){printf("%c", buffer[k]);}
                  printf("\n");
                } 
              }
          }
        }
      }
    }
  }
}
unsigned char * buscar_primer_espacio_vacio_pcb(int process_id) {
  unsigned char * virtual_dir;
  for (int i = 0; i < TAMANO_ENTRADA_PCB* N_ENTRADAS_PCB; i += TAMANO_ENTRADA_PCB)
  {
    if (i == 0 || !(i % TAMANO_ENTRADA_PCB)){
      if ((buffer[i + 1] == process_id) && (buffer[i])){
        int inicio = i + 14;
        for (int k = inicio; k < (i + 14 + 210); k += TAMANO_SUBENTRADA_PCB)
        {
          if (((k - inicio) == 0) || !((k - inicio) % (TAMANO_SUBENTRADA_PCB))){
            virtual_dir = malloc(4 * sizeof(unsigned char));
            for(int j = 0; j < 4; j++) virtual_dir[j] = buffer[k + j + 16];
            printf("Se encontró exitosamente una dirección virtual no ocupada para el archivo\n");
            return virtual_dir;
          }
        }
      }
    }
  }
  printf("Error: No se encontraron espacios libres para el proceso %d para crear un archivo en él\n", process_id);
}
CrmsFile * cr_open(int process_id, char * file_name, char mode){
  CrmsFile * archivo = malloc(sizeof(CrmsFile));
  int asignado = 0;
  for (int i = 0; i < TAMANO_ENTRADA_PCB * N_ENTRADAS_PCB; i += TAMANO_ENTRADA_PCB)
  {
    int errores = 0;
    int base, limit;
    if (i == 0 || !(i % TAMANO_ENTRADA_PCB)){
      if (buffer[i] && buffer[i + 1] == process_id){
        //printf("encontre el  proceso %d = %d \n", buffer[i+1], process_id);
        int inicio = i + 14; //donde empiezan las subentradas de archivos
        for (int j = inicio; j <= (i + 14 + 210); j += TAMANO_SUBENTRADA_PCB) //10 entradas de 21 bits cada una
          {//printf("j es: %d y inicio es %d\n", j, inicio);
            if ((j - inicio) == 0 || !((j - inicio) % TAMANO_SUBENTRADA_PCB)) //si estoy al inicio de una subentrada
              {
                //printf("ENTRADA\n");
                if (buffer[j] == 1)//si la subentrada es valida
                {
                  errores = 0;
                  for (int k = j, i_file = 0; k <= j + TAMANO_SUBENTRADA_PCB_NOMBRE_ARCHIVO; k++, i_file++){
                    if (buffer[k + 1] != file_name[i_file]){
                      errores += 1;
                    }
                  }
                  if (!errores && (mode == 'r')) {
                    // Si no hay errores (nombres iguales y pid iguales, entonces es el archivo buscado)
                    // Se escribe info en struct.
                    asignado = 1;
                    archivo -> nombre = malloc(12 * sizeof(unsigned char));
                    base = j;
                    limit = base + TAMANO_SUBENTRADA_PCB_NOMBRE_ARCHIVO;
                    archivo -> validez = buffer[base];
                    for (int k = base, i_file = 0; i_file < TAMANO_SUBENTRADA_PCB_NOMBRE_ARCHIVO && k <= limit; k++, i_file++){
                      archivo ->nombre[i_file] = buffer[k + 1];
                    }
                    archivo -> dir_virtual = malloc(TAMANO_SUBENTRADA_PCB_DIRECCION_VIRTUAL* sizeof(unsigned char));
                    base = base + TAMANO_SUBENTRADA_PCB_NOMBRE_ARCHIVO + TAMANO_SUBENTRADA_PCB_TAMANO_ARCHIVO;
                    limit = base + TAMANO_SUBENTRADA_PCB_DIRECCION_VIRTUAL;
                    for (int k = base, dir_counter = 0; i < limit && dir_counter < TAMANO_SUBENTRADA_PCB_DIRECCION_VIRTUAL; i++, dir_counter++)
                    {
                      archivo -> dir_virtual[dir_counter] = buffer[k + 1];
                    }
                    archivo -> tamano = malloc(TAMANO_SUBENTRADA_PCB_TAMANO_ARCHIVO* sizeof(unsigned char));
                    base = j + TAMANO_SUBENTRADA_PCB_NOMBRE_ARCHIVO;
                    limit = base + TAMANO_SUBENTRADA_PCB_TAMANO_ARCHIVO;
                    for (int k = base, dir_counter = 0; i < limit && dir_counter < TAMANO_SUBENTRADA_PCB_TAMANO_ARCHIVO; i++, dir_counter++)
                    {
                      archivo -> tamano[dir_counter] = buffer[k + 1];
                    }
                    printf("ARCHIVO POR LEER ENCONTRADO\n");
                    return archivo;
                    
                  }
                  else if (!errores && (mode == 'w')){
                    asignado = 1;
                  }
                  // Si no se cumple el if/elif de arriba, se pasa a la siguiente subentrada.
                } 
              }
          }
        // Ya se recorrieron todas las subentradas del proceso
        break;
      }
    }
  }
  if (!asignado && (mode == 'r')){
    printf("Error de lectura: el archivo con nombre %s no pudo ser encontrado en el proceso %d\n", file_name, process_id);
  }
  // ver lo del null terminator.
  else if ((!asignado) && (mode == 'w')) {
    // crear archivo
    archivo ->nombre = (unsigned char) file_name;
    archivo ->validez = 1;
    archivo ->tamano = 0;
    /* archivo ->dir_virtual */
    // Añadir direccion virtual
    archivo ->dir_virtual = buscar_primer_espacio_vacio_pcb(process_id);
    printf("SE CREO EL ARCHIVO REQUERIDO\n");
    return archivo;
  }
  else if (asignado && (mode == 'w')){
    printf("Error: El archivo que intenas escribir ya existe\n");
  }
}
int liberar_memoria_archivo(CrmsFile * archivo) {
  free(archivo ->nombre);
  free(archivo ->dir_virtual);
  free(archivo ->tamano);
  free(archivo);
  return 1;
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
  printf("-------Ejecutando la funcion cr_open-----------\n");
  CrmsFile * archivo = cr_open(200, "a.mp4", 'w');
  /* liberar_memoria_archivo(archivo); */
}