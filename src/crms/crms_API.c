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
    memory_file = malloc(sizeof(FILE));
    memory_file = (fopen(memory_path,"rb+"));
 }


/* Funcion que muestra en pantalla los procesos en ejecucion.*/
void cr_ls_processes()
  {
    fseek(memory_file, 0 ,SEEK_SET);
    fread(buffer,sizeof(buffer),1,memory_file); // read 10 bytes to our buffer*/
    int num = 1;
    int aux;
    for(int i = 0; i < N_ENTRADAS_PCB*TAMANO_ENTRADA_PCB; i += TAMANO_ENTRADA_PCB){
      if (i == 0 || !(i % TAMANO_ENTRADA_PCB)){
        if (buffer[i] == 1)
        {
          aux = i + 1;
          printf("\n [Entrada: %d, Proceso en ejecucion: %d] \n",num, buffer[aux]);
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
                  for (int l = j + TAMANO_SUBENTRADA_PCB_NOMBRE_ARCHIVO + TAMANO_SUBENTRADA_PCB_TAMANO_ARCHIVO; l < j + TAMANO_SUBENTRADA_PCB_NOMBRE_ARCHIVO + TAMANO_SUBENTRADA_PCB_TAMANO_ARCHIVO + TAMANO_SUBENTRADA_PCB_DIRECCION_VIRTUAL; l++)
                  {
                    printf("%u", buffer[l]);
                  }
                  
                  printf("\n");
                } 
              }
          }
        }
      }
    }
  }
}
// Retorna indice de primera subentrada vacia de PCB para un proceso process_id
int buscar_primer_espacio_vacio_pcb(int process_id) {
  unsigned char * virtual_dir;
  for (int i = 0; i < TAMANO_ENTRADA_PCB* N_ENTRADAS_PCB; i += TAMANO_ENTRADA_PCB)
  {
    if (i == 0 || !(i % TAMANO_ENTRADA_PCB)){
      if ((buffer[i + 1] == process_id) && (buffer[i])){
        int inicio = i + 14;
        for (int k = inicio; k < (i + 14 + 210); k += TAMANO_SUBENTRADA_PCB)
        {
          if (((k - inicio) == 0) || !((k - inicio) % (TAMANO_SUBENTRADA_PCB))){
            if (!buffer[k]){
              printf("Se encontró exitosamente una subentrada no ocupada para el archivo en el proceso %d\n", process_id);
              printf("%d\n", buffer[k]);
              return k;
            }
          }
        }
      }
    }
  }
  printf("Error: No se encontraron espacios libres para el proceso %d para crear un archivo en él\n", process_id);
  return 0;
}
int guardar_info_subentrada_a_struct(CrmsFile * archivo, int j){
  int base, limit;

  archivo -> nombre = malloc(TAMANO_SUBENTRADA_PCB_NOMBRE_ARCHIVO * sizeof(unsigned char));
  base = j;
  limit = base + TAMANO_SUBENTRADA_PCB_NOMBRE_ARCHIVO;
  archivo -> validez = buffer[base];
  for (int k = base, i_file = 0; i_file < TAMANO_SUBENTRADA_PCB_NOMBRE_ARCHIVO && k <= limit; k++, i_file++){
    archivo ->nombre[i_file] = buffer[k + 1];
  }

  archivo -> dir_virtual = malloc(TAMANO_SUBENTRADA_PCB_DIRECCION_VIRTUAL* sizeof(unsigned char));
  base = base + TAMANO_SUBENTRADA_PCB_NOMBRE_ARCHIVO + TAMANO_SUBENTRADA_PCB_TAMANO_ARCHIVO;
  limit = base + TAMANO_SUBENTRADA_PCB_DIRECCION_VIRTUAL;
  for (int k = base, dir_counter = 0; k < limit && dir_counter < TAMANO_SUBENTRADA_PCB_DIRECCION_VIRTUAL; k++, dir_counter++)
  {
    archivo -> dir_virtual[dir_counter] = buffer[k + 1];
  }

  archivo -> tamano = malloc(TAMANO_SUBENTRADA_PCB_TAMANO_ARCHIVO* sizeof(unsigned char));
  base = j + TAMANO_SUBENTRADA_PCB_NOMBRE_ARCHIVO;
  limit = base + TAMANO_SUBENTRADA_PCB_TAMANO_ARCHIVO;
  for (int k = base, dir_counter = 0; k < limit && dir_counter < TAMANO_SUBENTRADA_PCB_TAMANO_ARCHIVO; k++, dir_counter++)
  {
    archivo -> tamano[dir_counter] = buffer[k + 1];
  }
  return 1;
}

void guardar_info_new_file_a_archivo(CrmsFile * archivo, int idx_primer_indice_libre){
  char validation[4];
  int uno = 1;
  sprintf(validation, "%d", archivo ->validez);
  printf("%s\n", validation);

  // Escribir byte de validez.
  fseek(memory_file, (idx_primer_indice_libre) * sizeof(char) ,SEEK_SET);
  fwrite(&validation, sizeof(char), 1, memory_file);

  // Escribir nombre de crmsfile a archivo .bin
  for (int idx_nombre = 0; idx_nombre < TAMANO_SUBENTRADA_PCB_NOMBRE_ARCHIVO; idx_nombre++)
  {
    fseek(memory_file, 1 ,SEEK_CUR);
    fwrite(&(archivo -> nombre[idx_nombre]), 1, 1, memory_file);
  }
  // Escribir tamaño 0
  /* char tamano[4];
  sprintf(tamano, "%d", archivo ->tamano);
  printf("%s\n", tamano); */
  for (int idx_tamano = 0; idx_tamano < TAMANO_SUBENTRADA_PCB_TAMANO_ARCHIVO; idx_tamano++)
  {
    fseek(memory_file, sizeof(char) ,SEEK_CUR);
    fwrite(&(archivo ->tamano), sizeof(char), 1, memory_file);
  }
  fclose(memory_file);
}

CrmsFile * cr_open(int process_id, char * file_name, char mode){
  if (strlen(file_name) > TAMANO_SUBENTRADA_PCB_NOMBRE_ARCHIVO) {
    printf("Error: El nombre de archivo ingresado supera los 12 bytes");
    return;
  }
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
                    guardar_info_subentrada_a_struct(archivo, j);
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
    free(archivo);
  }
  // ver lo del null terminator.
  else if (asignado && (mode == 'w')){
    printf("Error: El archivo que intenas escribir ya existe\n");
    free(archivo);
  }
  else if ((!asignado) && (mode == 'w')) {
    // crear archivo
    int idx_primer_indice_libre = buscar_primer_espacio_vacio_pcb(process_id);
    if (!idx_primer_indice_libre) {
      printf("Error, no se encontró subentradas libres para ese proceso\n");
      free(archivo);
    }
    else {
      // Guardar nombre de file_name a struct archivo
      archivo ->nombre = calloc(TAMANO_SUBENTRADA_PCB_NOMBRE_ARCHIVO, sizeof(unsigned char));
      for (int i = 0; i < strlen(file_name) && i < TAMANO_SUBENTRADA_PCB_NOMBRE_ARCHIVO; i++)
      {
        archivo ->nombre[i] = (unsigned char) file_name[i];
      }

      // guardar validez y tamaño
      archivo ->validez = 1;
      archivo ->tamano = 0;
      // Añadir direccion virtual
      /* unsigned char * virtual_dir = malloc(4 * sizeof(unsigned char)); */
      archivo ->dir_virtual = malloc(4 * sizeof(unsigned char));
      // verificar este indice de inicio
      for(int j = 1; j <= 4; j++) {
        /* virtual_dir[j - 1] = buffer[idx_primer_indice_libre + j + 16];
        printf("%i-", buffer[idx_primer_indice_libre + j + 16]); */
        archivo ->dir_virtual[j-1] = 9;
      }
      /* archivo ->dir_virtual = virtual_dir; */
      guardar_info_new_file_a_archivo(archivo, idx_primer_indice_libre);
      printf("SE CREO EL ARCHIVO REQUERIDO\n");
      return archivo;
      printf("SE CREO EL ARCHIVO REQUERIDO\n");
    }
    
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
  int existe = cr_exists(200, "badcat.mp4");
  if (existe == 1){printf("El archivo SI esta almacenado en el proceso\n ");}
  else {printf("El archivo NO esta almacenado en el proceso\n ");}
  printf("\n");
  printf("-------Ejecutando la funcion cr__ls_files-----------\n");
  printf("\n");
  cr_ls_files(200);
  printf("-------Ejecutando la funcion cr_open-----------\n");
  CrmsFile * archivo = cr_open(200, "badcat.mp4", 'w');
  /* liberar_memoria_archivo(archivo); */
}