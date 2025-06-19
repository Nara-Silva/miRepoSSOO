#ifndef PLANIFICACION_H
#define PLANIFICACION_H

#include <stdbool.h>
#include <pthread.h>
#include "pcb.h"
#include "colas.h"
#include "memoria.h"

typedef enum { //Enum para los algoritmos de ordenamiento 
    FIFO,
    SJF_CON_DESALOJO,
    SJF_SIN_DESALOJO,
    PMCP
} algoritmo;

// =================== PLANIFICACION =========================
void planificador_fifo(t_pcb* nuevo_proceso);
void planificador_pmcp(t_pcb* nuevo_proceso);
void* planificador_largo_plazo(void* arg);
void* planificador_corto_plazo(void* arg);
void* planificador_mediano_plazo(void* arg);
void planificador_sjf_sin_desalojo(void);
void planificador_sjf_con_desalojo(void);
bool solicitar_inicializacion_memoria(t_pcb* proceso);
void enviar_finalizacion_memoria(t_pcb* pcb);
void enviar_a_ejecutar_a_cpu(t_pcb* pcb);
void enviar_interrupcion_cpu(int socket_cpu, int pid);
char* extraer_instruccion(void* buffer, int* desplazamiento);
void inicializar_cola(t_queue* cola_origen, pthread_mutex_t* mutex, status_cod origen, status_cod destino);
void intentar_inicializar_procesos_pendientes();
void* planificador_cortoplazo_fifo(void* arg);
void finalizar_proceso(t_pcb* proceso);
bool comparar_por_size(void* a, void* b);
bool comparar_por_tiempo_estimado(void* a, void* b);
void enviar_suspension_memoria(t_pcb* proceso);

// =====================IO========================