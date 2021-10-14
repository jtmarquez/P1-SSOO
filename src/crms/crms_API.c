#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h> 
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
    int cont = 0;
    int num = 1;
    int sum = 256;
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
  fclose(memory_file);
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
          printf("\nENTREEEEEEEEE\n");
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
              printf("VPN %d\n", vpn);
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
              printf("byte %d\n", byte_tabla);
              int pfn = 0;
              int validation = (byte_tabla >> 0) & 1;
              printf("validation %d\n" , validation);
              pfn += ((byte_tabla >> 7) & 1)*pow(2,0);
              pfn += ((byte_tabla >> 6) & 1)*pow(2,1);
              pfn += ((byte_tabla >> 5) & 1)*pow(2,2);
              pfn += ((byte_tabla >> 4) & 1)*pow(2,3);
              pfn += ((byte_tabla >> 3) & 1)*pow(2,4);
              pfn += ((byte_tabla >> 2) & 1)*pow(2,5);
              pfn += ((byte_tabla >> 1) & 1)*pow(2,6);
              
              printf("pfn %d\n", pfn);
              int byte_bitmap = floor(pfn/8);
              printf("byte index %d\n", byte_bitmap);
              unsigned char byte = buffer[byte_bitmap+4096];
              printf("byte %u\n", byte);
              int dif = pfn - byte_bitmap*8;
              printf("dif %u\n", dif);
              unsigned char byte_write = byte & (!(1 << dif));
              printf("byte write %u\n", byte_write);
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


void print_page_table(char* filename){
  cr_mount(filename);
  fseek(memory_file, 0 ,SEEK_SET);
  fread(buffer,sizeof(buffer),1,memory_file); // read 10 bytes to our buffer*/
  int inicio = 224;
  int cont = 224;
  int sum = 256;
  int num = 0;
  for (int j = 0; j <16; j+=1)
  {
    printf("\n [Proceso %d] \n", j);
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
    printf("i: %d validation %d: pfn %d\n", i, validation, pfn);
  }
    cont = inicio + j*sum;
  }
  
}


void print_frame_bitmap(char* filename){
  cr_mount(filename);
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
    printf("bit0 %d\n", bit0);
    int bit1 = ((buffer[i] >> 7) & 1)!=0;
    printf("bit1 %d\n", bit1);
    int bit2 = ((buffer[i] >> 6) & 1)!=0;
    printf("bit2 %d\n", bit2);
    int bit3 =  ((buffer[i] >> 5) & 1)!=0;
    printf("bit3 %d\n", bit3);
    int bit4 = ((buffer[i] >> 4) & 1)!=0;
    printf("bit4 %d\n", bit4);
    int bit5 =  ((buffer[i] >> 3) & 1)!=0;
    printf("bit5 %d\n", bit5);
    int bit6 =  ((buffer[i] >> 2) & 1)!=0;
    printf("bit6 %d\n", bit6);
    int bit7 =  ((buffer[i] >> 1) & 1)!=0;
    printf("bit7 %d\n", bit7);
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


int main(int argc, char **argv)
{
  printf("Hello P1!\n");
  char *input_name;
  input_name = argv[1];
  filename = input_name;
  print_page_table(filename);
  print_frame_bitmap(filename);
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
  cr_start_process(2, "test1");
  printf("\n");
  print_memory(filename);
  printf("-------Ejecutando la funcion cr_finish-----------\n");
  printf("\n");
  cr_finish_process(2);
  cr_finish_process(28);
  print_memory(filename);
}