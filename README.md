# P1-SSOO
Proyecto 1 SSOO 

Grupo: SegmentationFault

## Autores
- Javiera Irarrázabal: 17637740
- Francisca Otero: 17638097
- Elisa Fernandez: 17638003
- Jose Tomás Marquez: 1764111J
- Sofía Delano: 17639603

## Como correr la tarea?

1. ```make```
2. ```./crms <nombre archivo memoria>```

## Decisiones de diseño

- Se utilizaron structs solo para siertas estructuras, ya que no se encontró necesario para muchas. Para la mayoría de la tarea se va leyendo el archivo de memoria, guardándolo en buffer, y se va recorriendo este. Hay muchas funciones que sirven para imprimir y visualizar la memoria, para así tener un entendimiento general de lo que está pasando. 
- Se debe hacer ```cr_mount(filename)``` cada vez que el archivo sea cerrado en una función con ```fclose```.
- Se utiliza el comando ```memory_file[0] = *(fopen(memory_path,"rb+"));``` en cr_mount, el cual nos dimos cuenta que no sirve en Windows. Para correrla con windows se debe descomentar el comando ```memory_file = (fopen(memory_path,"rb+"));``` que se encuentra a continuación.

## Supuestos
