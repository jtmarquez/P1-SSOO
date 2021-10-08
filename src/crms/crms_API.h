#pragma once
#include <stdio.h>
#include "crms_API.h"

typedef struct Tabla_PCB
{
    struct Proceso* entradas[16];
};Tabla_PCB

struct Tabla_Paginas
{
    Paginas* paginas[32];
}tabla_paginas;

typedef struct Paginas
{
    unsigned char validez;
    unsigned char pfn;
};Paginas

struct Frame_bitmap
{
    unsigned char frames;
};

struct Frame
{
    
};

typedef struct Proceso
{
    unsigned char estado;
    unsigned char id;
    unsigned char nombre;
    CrmsFile* archivos[10];
    tabla_paginas;* Tabla_Paginas;
}Entrada;

typedef struct Archivos
{
    unsigned char validez;
    unsigned char nombre;
    unsigned char tamaño;
    unsigned char dir_virtual;   
}CrmsFile;