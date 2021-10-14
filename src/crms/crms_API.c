#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
// #include <byteswap.h>
#include <math.h>
#include "../file_manager/manager.h"
#include "crms_API.h"

#define N_ENTRADAS_PCB 16
#define TAMANO_ENTRADA_PCB 256
#define TAMANO_SUBENTRADA_PCB 21
#define TAMANO_SUBENTRADA_PCB_NOMBRE_ARCHIVO 12
#define TAMANO_SUBENTRADA_PCB_TAMANO_ARCHIVO 4
#define TAMANO_SUBENTRADA_PCB_DIRECCION_VIRTUAL 4 
#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c\n"
#define BYTE_TO_BINARY(byte)  \
  (byte & 0x80 ? '1' : '0'), \
  (byte & 0x40 ? '1' : '0'), \
  (byte & 0x20 ? '1' : '0'), \
  (byte & 0x10 ? '1' : '0'), \
  (byte & 0x08 ? '1' : '0'), \
  (byte & 0x04 ? '1' : '0'), \
  (byte & 0x02 ? '1' : '0'), \
  (byte & 0x01 ? '1' : '0') 

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
    // memory_file = (fopen(memory_path,"rb+"));
  }


/* Funcion que muestra en pantalla los procesos en ejecucion.*/
void cr_ls_processes()
  {
    fseek(memory_file, 0 ,SEEK_SET);
    fread(buffer,sizeof(buffer),1,memory_file); // read 10 bytes to our buffer*/
    int cont = 0;
    int num = 1;
    int sum = 256;
    int aux;
    for(int i = 0; i < N_ENTRADAS_PCB*TAMANO_ENTRADA_PCB; i += TAMANO_ENTRADA_PCB){
      if (i == 0 || !(i % TAMANO_ENTRADA_PCB)){
        if (buffer[i] == 1)
        {
          aux = i + 1;
          printf("\n [Entrada: %d, Proceso en ejecucion: %d] \n",num, buffer[aux]);
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

    for(int i = 0; i<4096; i++)
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
// Retorna indice de primera subentrada vacia de PCB para un proceso process_id
int buscar_primer_espacio_vacio_pcb(int process_id) {
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
int obtener_offset_archivo(unsigned char * bytes) {
  int offset = 0;
  offset += ((bytes[3] >> 7) & 1)*pow(2,0);
  offset += ((bytes[3] >> 6) & 1)*pow(2,1);
  offset += ((bytes[3] >> 5) & 1)*pow(2,2);
  offset += ((bytes[3] >> 4) & 1)*pow(2,3);
  offset += ((bytes[3] >> 3) & 1)*pow(2,4);
  offset += ((bytes[3] >> 2) & 1)*pow(2,5);
  offset += ((bytes[3] >> 1) & 1)*pow(2,6);
  offset += ((bytes[3] >> 0) & 1)*pow(2,7);
  offset += ((bytes[2] >> 7) & 1)*pow(2,8);
  offset += ((bytes[2] >> 6) & 1)*pow(2,9);
  offset += ((bytes[2] >> 5) & 1)*pow(2,10);
  offset += ((bytes[2] >> 4) & 1)*pow(2,11);
  offset += ((bytes[2] >> 3) & 1)*pow(2,12);
  offset += ((bytes[2] >> 2) & 1)*pow(2,13);
  offset += ((bytes[2] >> 1) & 1)*pow(2,14);
  offset += ((bytes[2] >> 0) & 1)*pow(2,15);
  offset += ((bytes[1] >> 7) & 1)*pow(2,16);
  offset += ((bytes[1] >> 6) & 1)*pow(2,17);
  offset += ((bytes[1] >> 5) & 1)*pow(2,18);
  offset += ((bytes[1] >> 4) & 1)*pow(2,19);
  offset += ((bytes[1] >> 3) & 1)*pow(2,20);
  offset += ((bytes[1] >> 2) & 1)*pow(2,21);
  offset += ((bytes[1] >> 1) & 1)*pow(2,22);
  return offset;
}

unsigned char obtener_VPN(CrmsFile* archivo){
  unsigned char p5 = (archivo ->dir_virtual[0] >> 3) & 0x1; // primer digito  (Byte >> x) AND 0x01
    unsigned char p51;
    unsigned char p9 = (archivo ->dir_virtual[1] >> 7) & 0x1;
    unsigned char mask = (1 << 7) - 0x01;

    if (p5) {
      p51 = p5 | (0x01 << 4);
    } else {
      p51 = p5 & (!(0x01 << 4));
    }
    unsigned char p6 = (archivo ->dir_virtual[0] >> 2) & 0x1; // segundo digito
    if (p6) {
      p51 = p51 | (0x01 << 3);
    } else {
      p51 = p51 & (!(0x01 << 3));
    }
    unsigned char p7 = (archivo ->dir_virtual[0] >> 1) & 0x1;; // tercer digito
    if (p7) {
      p51 = p51 | (0x01 << 2);
    } else {
      p51 = p51 & (!(0x01 << 2));
    }
    unsigned char p8 = (archivo ->dir_virtual[0] >> 0) & 0x1;; // cuarto digito
    if (p8) {
      p51 = p51 | (0x01 << 1);
    } else {
      p51 = p51 & (!(0x01 << 1));
    }
    if (p9){
      p51 = p51 | (0x01); // remplazar por 1
    } else {
      p51 = p51 & (0xFE); // reemplazar por 0
    }
  
  return p51;
}

int guardar_info_subentrada_a_struct(CrmsFile * archivo, int j){
  int base, limit;

  archivo -> nombre = malloc(TAMANO_SUBENTRADA_PCB_NOMBRE_ARCHIVO * sizeof(unsigned char));
  base = j;
  limit = base + TAMANO_SUBENTRADA_PCB_NOMBRE_ARCHIVO;
  archivo -> validez = buffer[base];

  for (int k = base, i_file = 0; i_file < TAMANO_SUBENTRADA_PCB_NOMBRE_ARCHIVO && k <= limit; k++, i_file++){
    archivo ->nombre[i_file] = buffer[k + 1];
    printf("%c", buffer[k+1]);
  }
  archivo -> dir_virtual = malloc(TAMANO_SUBENTRADA_PCB_DIRECCION_VIRTUAL* sizeof(unsigned char));
  base = base + TAMANO_SUBENTRADA_PCB_NOMBRE_ARCHIVO + TAMANO_SUBENTRADA_PCB_TAMANO_ARCHIVO;
  limit = base + TAMANO_SUBENTRADA_PCB_DIRECCION_VIRTUAL;
  int li = 0;
  for (int k = base, dir_counter = 0; k < limit && dir_counter < TAMANO_SUBENTRADA_PCB_DIRECCION_VIRTUAL; k++, dir_counter++)
  {
    li++;
    archivo -> dir_virtual[dir_counter] = buffer[k + 1];
    printf("%u %d\n",(unsigned char)buffer[k+1], dir_counter);
  }
  archivo -> vpn = obtener_VPN(archivo);
  archivo -> offset = obtener_offset_archivo(archivo ->dir_virtual);

  printf("Archivo VPN "BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(archivo -> vpn));
  printf("Archivo OFFSET "BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(archivo -> offset));

  archivo -> tamano = malloc(TAMANO_SUBENTRADA_PCB_TAMANO_ARCHIVO* sizeof(unsigned char));

  base = j + TAMANO_SUBENTRADA_PCB_NOMBRE_ARCHIVO;
  limit = base + TAMANO_SUBENTRADA_PCB_TAMANO_ARCHIVO;
  for (int k = base, dir_counter = 0; k < limit && dir_counter < TAMANO_SUBENTRADA_PCB_TAMANO_ARCHIVO; k++, dir_counter++)
  {
    archivo -> tamano[dir_counter] = buffer[k + 1];
  }
  unsigned int size = archivo -> tamano[3] | (archivo -> tamano[2] << 8) | (archivo -> tamano[1] << 16) | (archivo -> tamano[0] << 24);
  /* unsigned int l1 = bswap_32(archivo ->tamano); */
  /* printf("%u||||\n", size);
  printf("%u||!!!\n", l1); */
  /* printf("Leading text "BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(size));
  printf("Leading text "BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(l1)); */

  return 1;
}

int is_bigendian() 
{
  unsigned int i = 1;
  char *c = (char*)&i;
  if (*c){
    return 0;
  }
  return 1;
}
int obtener_dir_virtual_new_file(int idx_proceso, int idx_primer_indice_libre){

  // Esto está siendo revisado
  // Obtener el número de archivo, para acceder luego a esa posicion en la tabla de páginas.
  int n_archivo = (idx_primer_indice_libre - idx_proceso - 13)/21;
  // llegar a posición de tabla de páginas
  fseek(memory_file, (idx_proceso + 13 + 10*21 + n_archivo), SEEK_SET);
  unsigned char entrada_tabla_paginas;
  fread(&entrada_tabla_paginas, sizeof(unsigned char), 1, memory_file);
  printf("%c, %x\n",entrada_tabla_paginas, entrada_tabla_paginas);
  /* printf("%x|\n", bswap_32(entrada_tabla_paginas)); */
  /* if (!is_bigendian) entrada_tabla_paginas = bswap_32(entrada_tabla_paginas); */
  /* printf("%x||\n", (bswap_32(entrada_tabla_paginas) >> 7) & 0x01); */
  
  fseek(memory_file,0,SEEK_SET); 
  
  printf("%d %d %d\n", n_archivo, idx_primer_indice_libre, idx_proceso);

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

}
void guardar_info_new_file_a_archivo(CrmsFile * archivo, int idx_primer_indice_libre, int idx_proceso){
  // Esto está siendo revisado
  obtener_dir_virtual_new_file(idx_proceso, idx_primer_indice_libre);
  char validation[4];
  sprintf(validation, "%d", archivo ->validez);
  printf("%s\n", validation);
  // Escribir byte de validez.
  fseek(memory_file, (idx_primer_indice_libre) * sizeof(char) ,SEEK_SET);
  fwrite(&validation, sizeof(char), 1, memory_file);

  // Escribir nombre de crmsfile a archivo .bin
  for (int idx_nombre = 0; idx_nombre < TAMANO_SUBENTRADA_PCB_NOMBRE_ARCHIVO; idx_nombre++)
  {
    fwrite(&(archivo -> nombre[idx_nombre]), sizeof(char), 1, memory_file);
  }
  // Escribir tamaño 0
  for (int idx_tamano = 0; idx_tamano < TAMANO_SUBENTRADA_PCB_TAMANO_ARCHIVO; idx_tamano++)
  {
    fwrite(&(archivo ->tamano), sizeof(char), 1, memory_file);
  }
  /* fclose(memory_file); */
}

CrmsFile * cr_open(int process_id, char * file_name, char mode){
  if (strlen(file_name) > TAMANO_SUBENTRADA_PCB_NOMBRE_ARCHIVO) {
    printf("Error: El nombre de archivo ingresado supera los 12 bytes");
    return;
  }
  CrmsFile * archivo = malloc(sizeof(CrmsFile));
  int asignado = 0;
  int idx_proceso;
  int idx_subentrada = 0;
  for (int i = 0; i < TAMANO_ENTRADA_PCB * N_ENTRADAS_PCB; i += TAMANO_ENTRADA_PCB)
  {
    int errores = 0;

    if (i == 0 || !(i % TAMANO_ENTRADA_PCB)){
      if (buffer[i] && buffer[i + 1] == process_id){
        int inicio = i + 14; //donde empiezan las subentradas de archivos
        idx_proceso = i;
        for (int j = inicio; j <= (i + 14 + 210); j += TAMANO_SUBENTRADA_PCB) //10 entradas de 21 bits cada una
        {
          if ((j - inicio) == 0 || !((j - inicio) % TAMANO_SUBENTRADA_PCB)) //si estoy al inicio de una subentrada
          {
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
                printf("Se encontró exitosamente el archivo a leer\n");
                return archivo;
              }
              else if (!errores && (mode == 'w')){
                asignado = 1;
                idx_subentrada = j;
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
      archivo ->indice_buffer = idx_primer_indice_libre;
      
      if (n_entradas_validas == 0 && n_entradas_totales) {
        
      }
      // FALTA AÑADIR DIRECCION VIRTUAL
      // Dir fisica es: PFN + Offset
      // Obtener VPN -> 
      // Construir dir virtual: VPN + Offset

      guardar_info_new_file_a_archivo(archivo, idx_primer_indice_libre, idx_proceso);

      printf("Se creó exitosamente el archivo con nombre %s\n", archivo ->nombre);
      return archivo;
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
  printf("\n");
  printf("-------Ejecutando la funcion cr_ls_files-----------\n");
  cr_ls_files(200);
  printf("-------Ejecutando la funcion cr_exists-----------\n");
  printf("\n");
  int existe = cr_exists(200, "greatcat.mp4");
  if (existe){printf("si existe\n");}
  else{printf("no existe\n");}
  printf("\n");
  print_memory(filename);
  printf("-------Ejecutando la funcion cr_start-----------\n");
  printf("\n");
  cr_start_process(3, "test1");
  printf("\n");
  print_memory(filename);
  cr_ls_files(200);
  printf("-------Ejecutando la funcion cr_open-----------\n");
  CrmsFile * archivo = cr_open(200, "hecomes.mp4", 'r');
  /* liberar_memoria_archivo(archivo); */
}