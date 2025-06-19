#ifndef CPU_CICLO_INSTRUCCIONES_H
#define CPU_CICLO_INSTRUCCIONES_H

#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <commons/config.h>
#include <commons/string.h>
#include <commons/log.h>
#include <utils/instrucciones.h>
#include <utils/chiches.h>
#include <utils/protocolo.h>
#include <cpuUtils.h>

/* LO MOVÍ A instrucciones.h PQ LO NECESITO EN KERNEL
// Estructura para almacenar instruccion parseada
typedef struct {
    char* opcode;
    char* arg1;
    char* arg2;
} t_parsed_instruccion;
*/
typedef struct {
    int pid;
    int pagina;
    int marco;
    time_t ultimo_acceso; // solo si usamos LRU
} entrada_tlb;

typedef struct {
    int pid;
    int pagina;
    int marco;
    time_t ultimo_acceso; // solo si usás CLOCK-M o LRU
    bool modificada;     // solo si usás CLOCK-M
} entrada_cache;

typedef enum {
    FIFO,
    LRU
} t_algoritmo;

// Constantes para motivo de devolución a Kernel
#define MOTIVO_EXIT 1
#define MOTIVO_SYSCALL 2
#define MOTIVO_INTERRUPCION 3
#define MOTIVO_ERROR 4

// Constante para tamaño de memoria (debería venir de configuración)
#define TAMANIO_MEMORIA 1024

// Variables globales
extern int socket_kernel_dispatch;
extern int socket_kernel_interrupt;

// Funciones principales
void ejecutar_ciclo_instruccion(int socket_memoria, int pid, int pc);

// Funciones auxiliares internas 
char* fetch_instruccion(int socket_memoria, int pid, int pc);
bool decode_and_execute(char* instruccion_str, int pid, int pc, int socket_memoria);
bool check_interrupt(void);
void enviar_contexto_a_kernel(int pid, int pc, int motivo);

// Funciones de ejecución de instrucciones
void ejecutar_read(int pid, int direccion_logica, int tamanio, int socket_memoria);
void ejecutar_write(int pid, int direccion_logica, char* valor, int socket_memoria);
void enviar_syscall_a_kernel(int pid, int pc, char* instruccion_str, int motivo);
void loguear_instruccion(int pid, const char* instruccion_str);

#endif // CPU_CICLO_INSTRUCCIONES_H