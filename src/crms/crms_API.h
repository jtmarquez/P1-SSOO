#pragma once
#include <stdio.h>
#include "crms_API.h"

typedef struct Tabla_PCB
{
    struct Proceso* entradas[16];
}Tabla_PCB;

typedef struct Paginas
{
    unsigned char validez;
    unsigned char pfn;
}Paginas;

typedef struct Tabla_Paginas
{
    Paginas* paginas[32];
}TablaPaginas;



struct Frame_bitmap
{
    unsigned char frames;
};

struct Frame
{
    
};
typedef struct Archivos
{
    int indice_buffer;
    int id_proceso;
    unsigned char vpn;
    int offset;
    int creado;
    unsigned char validez;
    unsigned char * nombre;
    unsigned char * tamano;
    unsigned char * dir_virtual;   
    unsigned char * dir_fisica;
    int dir_virtual_int;
}CrmsFile;

typedef struct Proceso
{
    unsigned char estado;
    unsigned char id;
    unsigned char nombre;
    CrmsFile* archivos[10];
    TablaPaginas* T_P;

}Entrada;

