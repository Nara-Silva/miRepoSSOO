#ifndef CHICHES_H
#define CHICHES_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>

/**
* @brief Imprime un saludo por consola
* @param quien Módulo desde donde se llama a la función
* @return No devuelve nada
*/

#define CANTIDAD_ESTADOS 7 // Cantidad de estados posibles para un proceso

typedef enum {
    NEW,
    READY,
    EXEC,
    BLOCKED,
	SUSP_READY,
    SUSP_BLOCKED,
	SALIDA
} status_cod;

// PCB: estructura que contiene toda la informacion de un proceso (PCB de un proceso)
typedef struct {
    int pid;                    // Identificador del proceso (debera ser un numero entero, unico en todo el sistema que arranque en 0).
    int size;                   //tamanio del proceso
    int pc;                     // Program Counter, es un numero entero que arranca en 0.
    int metricas_estado [CANTIDAD_ESTADOS];    // listado de estados contabilizando la cantidad de veces que el proceso estuvo en cada uno de ellos.
    int metricas_tiempo [CANTIDAD_ESTADOS]; // listado con los tiempos en los que permanecio el proceso en cada estado. UTILIZAMOS ARRAYS EN VEZ DE LISTAS, YA QUE TIENEN UNA CANTIDAD FIJA DE ELEMENTOS
    status_cod estado;          // Estado del proceso
    time_t tiempo_inicio_estado; // Tiempo de inicio del estado actual
    int tiempo_estimado;        // Tiempo estimado de ejecución para SJF/SRT
    int tiempo_restante;        // Tiempo restante de ejecución para SRT
    int tiempo_bloqueado;       // Tiempo que lleva bloqueado
    char* archivo_instrucciones; // Archivo de pseudocódigo
} t_pcb;

// COLAS DE ESTADOS
extern t_queue* cola_new;
extern t_queue* cola_ready;
extern t_queue* cola_blocked;
extern t_queue* cola_exe;
extern t_queue* cola_exit;
extern t_queue* cola_susp_ready;
extern t_queue* cola_susp_blocked;


void saludar(char* quien);
int crear_conexion(char* ip, char* puerto);
int crear_servidor(char* puerto);
int esperar_cliente (int fd_escucha, char* cliente_nombre, t_log* logger);
void terminar_programa(int conexion, t_log* logger, t_config* config);
int generar_handshake(int fd, t_log* logger);
void enviarHandshake(int fd);

#endif
