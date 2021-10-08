#include <stdbool.h>
// Tells the compiler to compile this file once
#pragma once

// Define compile-time constants
#define MAX_SPLIT 255
#define BUFFER_SIZE 4096

// Define the struct
typedef struct inputfile
{
  int len;
  char ***lines; // This is an array of arrays of strings
} InputFile;

// Declare functions
InputFile *read_file(char *filename);
void input_file_destroy(InputFile *input_file);

// Estructuras propias

// Struct estados
typedef enum process_status
{
  RUNNING,
  WAITING,
  READY,
  FINISHED,
} ProcessStatus;

// Struct Proceso
typedef struct process
{
  int tournaround_time;
  int init_time;        // tiempo en que se creara proceso
  char *name;           // puntero al nombre del proceso
  int factory;          // fabrica a la que pertenece el proceso
  int number_of_bursts; // numbero de rafagas que viene en input
  int *bursts;          // puntero a cada Ai y Bi
  int time_passed;      // pensado para tiempo que ha pasado desde que se emprezo Ai o Bi
  int actual_burst;
  bool in_queue;
  bool quantum_interrupted; // numero correspondiente a posicion Ai o Bi en *burst para saber en cual se esta      // si proceso esta en cola o no
  int number_in_cpu;
  int number_of_interruptions;
  int waiting_time;
  int response_time;
  ProcessStatus status;     // estado del proceso
} Process;

// struct Queue
typedef struct queue
{
  int *queue; // puntero a los procesos que estan en cola
} Queue;