#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h> 
#include "../file_manager/manager.h"
#include "crms_API.h"
#include <byteswap.h>

#define N_ENTRADAS_PCB 16
#define TAMANO_ENTRADA_PCB 256
#define TAMANO_SUBENTRADA_PCB 21
#define TAMANO_SUBENTRADA_PCB_NOMBRE_ARCHIVO 12
#define TAMANO_SUBENTRADA_PCB_TAMANO_ARCHIVO 4
#define TAMANO_SUBENTRADA_PCB_DIRECCION_VIRTUAL 4 
#define TAMANO_SUBENTRADA_FRAME 8388608
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
FILE * variable_a_hacer_free;
char *memory_local_path;
unsigned char buffer[5000];

/*Funcion para montar la memoria.
  Establece como variable global la ruta local donde se encuentra el archivo .bin 
  correspondiente a la memoria.*/


 void cr_mount(char* memory_path)
  {
    memory_local_path = memory_path;
    memory_file = malloc(sizeof(FILE));
    variable_a_hacer_free = memory_file;
    memory_file = (fopen(memory_path,"rb+"));
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
        int uno = 1;
        fseek(memory_file, i*sizeof(char),SEEK_SET);
        fwrite(&uno, sizeof(char), 1, memory_file);
        fseek(memory_file, (i+1)*sizeof(char),SEEK_SET);
        fwrite(&process_id, sizeof(char), 1, memory_file);
        for (int j = 0; j < strlen(process_name); j++)
        {
          char letter = (char)process_name[j];
          fseek(memory_file, (i+2+j)*sizeof(char),SEEK_SET);
          fwrite(&letter, sizeof(char), 1, memory_file);

        }
        continue;
      }
    cont += sum;
    }
  }
  /* fclose(memory_file); */
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

unsigned char obtener_VPN(unsigned char pos_0_dir_virtual, unsigned char pos_1_dir_virtual){
  unsigned char p5 = (pos_0_dir_virtual >> 3) & 0x1; // primer digito  (Byte >> x) AND 0x01
  unsigned char p51;
  unsigned char p9 = (pos_1_dir_virtual >> 7) & 0x1;
  unsigned char mask = (1 << 7) - 0x01;
  if (p5) {
    p51 = p5 | (0x01 << 4);
  } else {
    p51 = p5 & (!(0x01 << 4));
  }
  unsigned char p6 = (pos_0_dir_virtual >> 2) & 0x1; // segundo digito
  if (p6) {
    p51 = p51 | (0x01 << 3);
  } else {
    p51 = p51 & (!(0x01 << 3));
  }
  unsigned char p7 = (pos_0_dir_virtual >> 1) & 0x1;; // tercer digito
  if (p7) {
    p51 = p51 | (0x01 << 2);
  } else {
    p51 = p51 & (!(0x01 << 2));
  }
  unsigned char p8 = (pos_0_dir_virtual >> 0) & 0x1;; // cuarto digito
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

unsigned char get_last_n_bits(unsigned char byte, int n){
  // queremos obtener los 7 últimos bits.
  unsigned char mask = (1 << n) - 0x01;
  unsigned char pfn = byte & mask;
  return pfn;
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
  archivo -> vpn = obtener_VPN(archivo ->dir_virtual[0], archivo ->dir_virtual[1]);
  archivo -> offset = obtener_offset_archivo(archivo ->dir_virtual);

  /* printf("Archivo VPN "BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(archivo -> vpn));
  printf("Archivo OFFSET "BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(archivo -> offset)); */

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
  /* printf("%c, %x\n",entrada_tabla_paginas, entrada_tabla_paginas); */
  /* printf("%x|\n", bswap_32(entrada_tabla_paginas)); */
  /* if (!is_bigendian) entrada_tabla_paginas = bswap_32(entrada_tabla_paginas); */
  /* printf("%x||\n", (bswap_32(entrada_tabla_paginas) >> 7) & 0x01); */
  
  fseek(memory_file,0,SEEK_SET); 
  
  printf("%d %d %d\n", n_archivo, idx_primer_indice_libre, idx_proceso);
}
void cr_finish_process(int process_id) 
{
  fseek(memory_file, 0 ,SEEK_SET);
  fread(buffer,sizeof(buffer),1,memory_file);
  int cont = 0;
  int sum = 256;
  int cero = 0;

  for(int i = 0; i < N_ENTRADAS_PCB*TAMANO_ENTRADA_PCB; i++)
  {
    if (i == cont){ //si estoy al inicio de una de las entradas
      if (buffer[i] == 1) //si el proceso esta en ejecucion (bit validez = 1)
      {
        if (buffer[i+1] == process_id) //si encuentro el proceso correspondiente
        {
          /* printf("\nENTREEEEEEEEE\n"); */
          fseek(memory_file, i*sizeof(char),SEEK_SET);
          fwrite(&cero, sizeof(char), 1, memory_file); //invalido el proceso correspondiente
          // INVALIDAR PROCESO EN FRAME BITMAP
          // 10 entradas archivos
          for (int k = 0; k < 10*21; k+=21)
          {
            if (buffer[i+14+k] == 1)
            {
              // printf("lugar validez  \n%d\n", i+14+k);
              fseek(memory_file, (i+14+k)*sizeof(char),SEEK_SET);
              fwrite(&cero, sizeof(char), 1, memory_file); //invalido el archivo correspondiente
              // dirección virtual
              // printf("\n");
              // for (int a = i+14+k; a <= i+14+k+12; a++)
              // {
              //   printf("%c", buffer[a]);
              // }
              // printf("\n");
              
              unsigned char bytes[4];
              for (int j = 0; j < 4; j++)
              {
                bytes[j] = buffer[i+31+k+j];
                // printf("lugar i byte  \n%d\n", i+31+k+j);
              };
              // printf("byte 0 %u\n", (bytes[0]));
              // printf("byte 1 %u\n", (bytes[1]));
              // printf("byte 2 %u\n", (bytes[2]));
              // printf("byte 3 %u\n", (bytes[3]));
              // printf("bit 0 \n%u\n", (bytes[0] >> 4) & 1);
              // printf("bit 1 \n%u\n", (bytes[0] >> 5) & 1);
              // printf("bit 2 \n%u\n", (bytes[0] >> 6) & 1);
              // printf("bit 3 \n%u\n", (bytes[0] >> 7) & 1);
              // printf("bit 4 \n%u\n", (bytes[1] >> 0) & 1);
              int vpn = 0;
              vpn += ((bytes[1] >> 0) & 1)*pow(2,0);
              vpn += ((bytes[0] >> 7) & 1)*pow(2,1);
              vpn += ((bytes[0] >> 6) & 1)*pow(2,2);
              vpn += ((bytes[0] >> 5) & 1)*pow(2,3);
              vpn += ((bytes[0] >> 4) & 1)*pow(2,4);
              /* printf("VPN %d\n", vpn); */
              // int offset = 0;
              // offset += ((bytes[3] >> 7) & 1)*pow(2,0);
              // offset += ((bytes[3] >> 6) & 1)*pow(2,1);
              // offset += ((bytes[3] >> 5) & 1)*pow(2,2);
              // offset += ((bytes[3] >> 4) & 1)*pow(2,3);
              // offset += ((bytes[3] >> 3) & 1)*pow(2,4);
              // offset += ((bytes[3] >> 2) & 1)*pow(2,5);
              // offset += ((bytes[3] >> 1) & 1)*pow(2,6);
              // offset += ((bytes[3] >> 0) & 1)*pow(2,7);
              // offset += ((bytes[2] >> 7) & 1)*pow(2,8);
              // offset += ((bytes[2] >> 6) & 1)*pow(2,9);
              // offset += ((bytes[2] >> 5) & 1)*pow(2,10);
              // offset += ((bytes[2] >> 4) & 1)*pow(2,11);
              // offset += ((bytes[2] >> 3) & 1)*pow(2,12);
              // offset += ((bytes[2] >> 2) & 1)*pow(2,13);
              // offset += ((bytes[2] >> 1) & 1)*pow(2,14);
              // offset += ((bytes[2] >> 0) & 1)*pow(2,15);
              // offset += ((bytes[1] >> 7) & 1)*pow(2,16);
              // offset += ((bytes[1] >> 6) & 1)*pow(2,17);
              // offset += ((bytes[1] >> 5) & 1)*pow(2,18);
              // offset += ((bytes[1] >> 4) & 1)*pow(2,19);
              // offset += ((bytes[1] >> 3) & 1)*pow(2,20);
              // offset += ((bytes[1] >> 2) & 1)*pow(2,21);
              // offset += ((bytes[1] >> 1) & 1)*pow(2,22);
              // printf("OFFSET %d\n", offset);
              unsigned char byte_tabla = buffer[i+224+vpn];
              /* printf("byte %d\n", byte_tabla); */
              int pfn = 0;
              int validation = (byte_tabla >> 0) & 1;
              /* printf("validation %d\n" , validation); */
              pfn += ((byte_tabla >> 7) & 1)*pow(2,0);
              pfn += ((byte_tabla >> 6) & 1)*pow(2,1);
              pfn += ((byte_tabla >> 5) & 1)*pow(2,2);
              pfn += ((byte_tabla >> 4) & 1)*pow(2,3);
              pfn += ((byte_tabla >> 3) & 1)*pow(2,4);
              pfn += ((byte_tabla >> 2) & 1)*pow(2,5);
              pfn += ((byte_tabla >> 1) & 1)*pow(2,6);
              
              /* printf("pfn %d\n", pfn); */
              int byte_bitmap = floor(pfn/8);
              /* printf("byte index %d\n", byte_bitmap); */
              unsigned char byte = buffer[byte_bitmap+4096];
              /* printf("byte %u\n", byte); */
              int dif = pfn - byte_bitmap*8;
              /* printf("dif %u\n", dif); */
              unsigned char byte_write = byte & (!(1 << dif));
              /* printf("byte write %u\n", byte_write); */
              //fseek(memory_file, (byte_bitmap+4000)*sizeof(char),SEEK_SET);
              //fwrite(&byte_write, sizeof(char), 1, memory_file); //invalido el archivo correspondiente


            };
          }
        }
      }
    cont += sum;
    }
  }
}

void print_memory(char* filename){
  /* cr_mount(filename); */
  fseek(memory_file, 0 ,SEEK_SET);
  fread(buffer,sizeof(buffer),1,memory_file); // read 10 bytes to our buffer*/
  int cont = 0;
  int sum = 256;
  int num = 1;

  for(int i = 0; i < N_ENTRADAS_PCB*TAMANO_ENTRADA_PCB; i++){
    if (i == cont){ 
      /* printf("\n [Entrada: %d] \n",num); */
      cont += sum;
      num +=1;
    }
    /* printf("%d", buffer[i]); */
  }
}

void print_page_table(char* filename){
  /* cr_mount(filename); */
  fseek(memory_file, 0 ,SEEK_SET);
  fread(buffer,sizeof(buffer),1,memory_file); // read 10 bytes to our buffer*/
  int inicio = 224;
  int cont = 224;
  int sum = 256;
  int num = 0;
  for (int j = 0; j <16; j+=1)
  {
    /* printf("\n [Proceso %d] \n", j); */
    for(int i = (inicio + j*sum); i < (inicio + 32+ j*sum); i++){
  
    int pfn = 0;
    int validation = ((buffer[i] >> 0) & 1) !=0; //primer bit
    pfn += ((buffer[i] >> 7) & 1)*pow(2,0);
    pfn += ((buffer[i] >> 6) & 1)*pow(2,1);
    pfn += ((buffer[i] >> 5) & 1)*pow(2,2);
    pfn += ((buffer[i] >> 4) & 1)*pow(2,3);
    pfn += ((buffer[i] >> 3) & 1)*pow(2,4);
    pfn += ((buffer[i] >> 2) & 1)*pow(2,5);
    pfn += ((buffer[i] >> 1) & 1)*pow(2,6);
    /* printf("i: %d validation %d: pfn %d\n", i, validation, pfn); */
  }
    cont = inicio + j*sum;
  }
}


void print_frame_bitmap(char* filename){
  /* cr_mount(filename); */
  fseek(memory_file, 0 ,SEEK_SET);
  fread(buffer,sizeof(buffer),1,memory_file); // read 10 bytes to our buffer*/
  int inicio = 4096;
  int cont = 4096;
  int num = 0;

  for(int i = inicio; i < inicio + 16; i++){
    if (i == cont){ 
        printf("\n [Entrada: %d] \n",num);
        
    cont += 1;
    num +=1;
    }
    int bit0 =  ((buffer[i] >> 0) & 1) !=0; //primer bit
    /* printf("bit0 %d\n", bit0); */
    int bit1 = ((buffer[i] >> 7) & 1)!=0;
    /* printf("bit1 %d\n", bit1); */
    int bit2 = ((buffer[i] >> 6) & 1)!=0;
    /* printf("bit2 %d\n", bit2); */
    int bit3 =  ((buffer[i] >> 5) & 1)!=0;
    /* printf("bit3 %d\n", bit3); */
    int bit4 = ((buffer[i] >> 4) & 1)!=0;
    /* printf("bit4 %d\n", bit4); */
    int bit5 =  ((buffer[i] >> 3) & 1)!=0;
    /* printf("bit5 %d\n", bit5); */
    int bit6 =  ((buffer[i] >> 2) & 1)!=0;
    /* printf("bit6 %d\n", bit6); */
    int bit7 =  ((buffer[i] >> 1) & 1)!=0;
    /* printf("bit7 %d\n", bit7); */
  }
}




/*Funcion para liberar memoria de un archivo perteneciente al proceso con id process id. 
Para esto el archivo debe dejar de aparecer en la memoria virtual del proceso, 
ademas, si los frames quedan totalmente libres se debe indicar en el frame bitmap 
que ese frame ya no esta siendo utilizado e invalidar la entrada correspondiente en la tabla de paginas.*/

// void cr_delete_file(CrmsFile* file_desc)
//   {
//     fseek(memory_file, 0 ,SEEK_SET); //me paro al inicio de la memoria
//     fread(buffer,sizeof(buffer),1,memory_file); //cargo la informacion del archivo en un buffer
//     int inicio_archivo = file_desc.indice_buffer;//encuentro el inicio del archivo que quiero
//     int zero = 0;
//     fseek(memory_file, inicio_archivo*sizeof(char),SEEK_SET);
//     printf("pos inicio archivo es: %d\n ", inicio_archivo);
//     printf("ahi encontre: %d\n ", buffer[inicio_archivo]);
//     fwrite(&zero, sizeof(char), 1, memory_file); //pongo en 0 el bit de validez del archivo en mem virtual
//     //me paro en la direccion virtual
//     unsigned char bytes[4]; //para los 4 bytes de la dir virtual
//     for (int i = 0; i <4; i+=1)
//       {
//         bytes[i] = buffer[inicio_archivo + 17 + i] //bytes de dir virtual
//       }
//     int vpn = 0; //vpn segundos 4 bits de primer byte + primer bit del primer byte
//     vpn += (((bytes[1] >> 0) & 1)!=0) *pow(2,0);
//     vpn += (((bytes[0] >> 7) & 1)!=0)*pow(2,1);
//     vpn += (((bytes[0] >> 6) & 1)!=0)*pow(2,2);
//     vpn += (((bytes[0] >> 5) & 1)!=0)*pow(2,3);
//     vpn += (((bytes[0] >> 4) & 1)!=0)*pow(2,4);
//     printf("VPN %d\n", vpn); //obtengo el vpn

//     unsigned char byte_tabla_pags = buffer[4096 + vpm] -> pagina;
//     printf("byte %d\n", byte_tabla);

//     //obtengo el pfn
//     int pfn = 0;
//     int validation = ((byte_tabla >> 0) & 1) !=0; //primer bit
//     printf("validation %d\n" , validation);
//     pfn += ((byte_tabla >> 7) & 1)*pow(2,0);
//     pfn += ((byte_tabla >> 6) & 1)*pow(2,1);
//     pfn += ((byte_tabla >> 5) & 1)*pow(2,2);
//     pfn += ((byte_tabla >> 4) & 1)*pow(2,3);
//     pfn += ((byte_tabla >> 3) & 1)*pow(2,4);
//     pfn += ((byte_tabla >> 2) & 1)*pow(2,5);
//     pfn += ((byte_tabla >> 1) & 1)*pow(2,6);
//     printf("pfn %d\n", pfn);

//     /*int byte_bitmap = floor(pfn/8); //encuentro en que byte esta el frame
//     printf("byte index %d\n", byte_bitmap);
//     unsigned char byte = buffer[byte_bitmap+4000];
//     printf("byte %u\n", byte);
//     int dif = pfn - byte_bitmap*8;
//     printf("dif %u\n", dif);
//     unsigned char byte_write = byte & (!(1 << dif));
//     printf("byte write %u\n", byte_write);
//     //fseek(memory_file, (byte_bitmap+4000)*sizeof(char),SEEK_SET);
//     //fwrite(&byte_write, sizeof(char), 1, memory_file); //invalido el archivo correspondiente
//     //obtengo la dir fisica*/
//   }


void guardar_nombre_y_tamano_archivo(CrmsFile * archivo, int idx_primer_indice_libre){
  // Escribir nombre de crmsfile a archivo .bin
  fseek(memory_file, idx_primer_indice_libre ,SEEK_SET); //me paro al inicio de la memoria
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
int encontrar_idx_ultimo_archivo(int idx_primer_archivo_libre, int idx_proceso) {
  int inicio = idx_proceso + 14;
  int ultimo_archivo_idx = 0;
  for (int i = inicio; i < (inicio + 210); i += TAMANO_SUBENTRADA_PCB)
  {
    if ((i - inicio) == 0 || !((i - inicio) % TAMANO_SUBENTRADA_PCB)) //si estoy al inicio de una subentrada
    {
      if (buffer[i] == 1){
        ultimo_archivo_idx = i;
      }
      else if ((buffer[i] == 0) && (i == idx_primer_archivo_libre)){
        // encontré el espacio libre, deberia tener un idx para el último archivo.
        // Si no lo tengo (ultimo_archivo_idx = 0), entonces vpn = 0;

        return ultimo_archivo_idx;
      }
    }
  }
}
unsigned char encontrar_vpn_ultimo_archivo_mas_1(int idx_primer_archivo_libre, int idx_proceso){
  // idx_proceso apunta a bit de validez del proceso
  // 14 bits
  fseek(memory_file, 0 ,SEEK_SET); //me paro al inicio de la memoria
  fread(buffer,sizeof(buffer),1,memory_file); //cargo la informacion del archivo en un buffer

  int ultimo_archivo_idx = encontrar_idx_ultimo_archivo(idx_primer_archivo_libre, idx_proceso);
  if (!ultimo_archivo_idx){
    return 0x0;
  } else {

    /* int limit = ultimo_archivo_idx + TAMANO_SUBENTRADA_PCB_NOMBRE_ARCHIVO + TAMANO_SUBENTRADA_PCB_TAMANO_ARCHIVO + TAMANO_SUBENTRADA_PCB_DIRECCION_VIRTUAL; */
    int base = ultimo_archivo_idx + TAMANO_SUBENTRADA_PCB_NOMBRE_ARCHIVO + TAMANO_SUBENTRADA_PCB_TAMANO_ARCHIVO;
    
    unsigned char vpn = obtener_VPN(buffer[base], buffer[base + 1]);
    return vpn + 0x01;
    /* for (int i = base; i < limit; i++)
    {
      unsigned char vpn = obtener_VPN(buffer[i], buffer[i + 1]);
      return vpn + 0x01;
    } */
  }
}
unsigned char * get_direccion_virtual(unsigned char vpn, unsigned char offset){
  unsigned char * dir_vir = malloc(TAMANO_SUBENTRADA_PCB_DIRECCION_VIRTUAL * sizeof(unsigned char));
  dir_vir[0] = (0b0000 << 4) | (vpn >> 1);
  dir_vir[1] = (get_last_n_bits(vpn, 1) << 8) | (offset >> 16);
  dir_vir[2] = get_last_n_bits((offset >> 8), 8);
  dir_vir[3] = get_last_n_bits(offset, 8);
  return dir_vir;
}
unsigned int encontrar_tamano_ultimo_archivo(int idx_primer_indice_libre, int idx_proceso){
  // idx_proceso apunta a bit de validez del proceso
  // 14 bits
  fseek(memory_file, 0 ,SEEK_SET); //me paro al inicio de la memoria
  fread(buffer,sizeof(buffer),1,memory_file); //cargo la informacion del archivo en un buffer

  int ultimo_archivo_idx = encontrar_idx_ultimo_archivo(idx_primer_indice_libre, idx_proceso);
  if (!ultimo_archivo_idx){
    return 0x0;
  } else {

    int base = ultimo_archivo_idx + TAMANO_SUBENTRADA_PCB_NOMBRE_ARCHIVO;
    int limit = ultimo_archivo_idx + TAMANO_SUBENTRADA_PCB_NOMBRE_ARCHIVO + TAMANO_SUBENTRADA_PCB_TAMANO_ARCHIVO;
    unsigned char tamano[4];
    for (int i = base, counter = 0; i < limit; i++, counter++)
    {
      tamano[counter] = buffer[i];
    }
    
    // Ver esto de bswap
    unsigned int tamano_int = (unsigned int)bswap_32(tamano);
    return tamano_int;
  }
}
void guardar_bit_validez(CrmsFile * archivo, int idx_subentrada_archivo, int vpn, int idx_proceso, int pfn){
  /* unsigned char byte_validez = 0x01; */
  int byte_validez = 1;
  // Primero guardamos byte de validez en tabla de PCBS

  //me paro en el byte de validez de la subentrada
  fseek(memory_file, idx_subentrada_archivo ,SEEK_SET); 

  // Escribimos en byte idx_subentrada_archivo
  fwrite(&byte_validez, sizeof(int), 1, memory_file);

  // Ahora guardamos byte en tabla de páginas
  int idx_subentrada_tabla_paginas = idx_proceso + 1 + 12 + 10 * TAMANO_SUBENTRADA_PCB + vpn;

  fseek(memory_file, idx_subentrada_tabla_paginas, SEEK_SET);
  fwrite(&byte_validez, sizeof(int), 1, memory_file);

  // Ahora guardamos byte en frame bitmap
  int idx_subentrada_bitmap = 16 * TAMANO_ENTRADA_PCB + pfn;
  fseek(memory_file, idx_subentrada_bitmap, SEEK_SET);
  fwrite(&byte_validez, sizeof(int), 1, memory_file);
  /* fclose(memory_file); */
}
CrmsFile * cr_open(int process_id, char * file_name, char mode){
  CrmsFile * archivo = malloc(sizeof(CrmsFile));
  archivo -> creado = 1;
  if (strlen(file_name) > TAMANO_SUBENTRADA_PCB_NOMBRE_ARCHIVO) {
    printf("Error: El nombre de archivo ingresado supera los 12 bytes");
    archivo ->creado = 0;
    return archivo;
  }
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
    archivo ->creado = 0;
    return archivo;
  }

  else if (asignado && (mode == 'w')){
    printf("Error: El archivo que intenas escribir ya existe\n");
    archivo ->creado = 0;
    return archivo;
  }

  else if ((!asignado) && (mode == 'w')) {
    // crear archivo
    int idx_primer_indice_libre = buscar_primer_espacio_vacio_pcb(process_id);

    if (!idx_primer_indice_libre) {
      printf("Error, no se encontró subentradas libres para ese proceso\n");
      archivo ->creado = 0;
      return archivo;
    }
    else {
      // Guardar nombre de file_name a struct archivo
      archivo ->nombre = calloc(TAMANO_SUBENTRADA_PCB_NOMBRE_ARCHIVO, sizeof(unsigned char));
      for (int i = 0; i < strlen(file_name) && i < TAMANO_SUBENTRADA_PCB_NOMBRE_ARCHIVO; i++)
      {
        archivo ->nombre[i] = (unsigned char) file_name[i];
      }
      archivo ->validez = 1;
      archivo ->tamano = 0x0;
      archivo ->indice_buffer = idx_primer_indice_libre;
      unsigned char vpn = encontrar_vpn_ultimo_archivo_mas_1(idx_primer_indice_libre, idx_proceso);
      unsigned int tamano = encontrar_tamano_ultimo_archivo(idx_primer_indice_libre, idx_proceso);
      guardar_nombre_y_tamano_archivo(archivo, idx_primer_indice_libre);
      int offset = (tamano >= TAMANO_SUBENTRADA_FRAME) ? (tamano - TAMANO_SUBENTRADA_FRAME) : tamano;
      if (tamano >= TAMANO_SUBENTRADA_FRAME) {
        offset = tamano - TAMANO_SUBENTRADA_FRAME;
        vpn++;
      } else {
        offset = tamano;
      }
      printf("Archivo VPN "BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(vpn));
      int vpn_int = (int) vpn;
      unsigned char byte = buffer[idx_proceso + 14 + 10*TAMANO_SUBENTRADA_PCB + vpn_int];
      unsigned char pfn = get_last_n_bits(byte, 7);
      printf("Archivo VPN "BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(pfn));
      printf("pfn: %i\n", (int)pfn);

      archivo ->dir_virtual = get_direccion_virtual(vpn, offset);
      // falta escribir a archivo la dirección virtual
      guardar_bit_validez(archivo, idx_subentrada, vpn_int, idx_proceso, (int) pfn);
      printf("Se creó exitosamente el archivo con nombre %s\n", archivo ->nombre);
      return archivo;
    }
    
  }
}

int liberar_memoria_archivo(CrmsFile * archivo) {
  if (!archivo ->creado) {
    free(archivo);
  } else {
    free(archivo ->nombre);
    free(archivo ->dir_virtual);
    free(archivo ->tamano);
    free(archivo);
  }
  return 1;
}

int main(int argc, char **argv)
{
  printf("Hello P1!\n");
  char *input_name;
  input_name = argv[1];
  filename = input_name;
  cr_mount(filename);
  print_page_table(filename);
  print_frame_bitmap(filename);
  printf("\n");
  printf("-------Ejecutando la funcion cr_mount-------\n");
  printf("\n");
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
  int existe = cr_exists(200, "bichota.mp4");
  if (existe){printf("si existe\n");}
  else{printf("no existe\n");}
  printf("\n");
  print_memory(filename);
  printf("-------Ejecutando la funcion cr_start-----------\n");
  printf("\n");
  cr_start_process(2, "test1");
  printf("\n");
  print_memory(filename);
  printf("-------Ejecutando la funcion cr_finish-----------\n");
  printf("\n");
  cr_finish_process(2);
  cr_finish_process(28);
  print_memory(filename);
  cr_ls_files(200);
  printf("-------Ejecutando la funcion cr_open-----------\n");
  CrmsFile * archivo = cr_open(200, "bichota.mp4", 'w');
  liberar_memoria_archivo(archivo);
  fclose(memory_file);
  free(variable_a_hacer_free);
}