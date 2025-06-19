#ifndef COLAS_H
#define COLAS_H

#include <pthread.h>
#include <semaphore.h>
#include "pcb.h"         // Para t_pcb
#include <commons/collections/queue.h>
#include <commons/log.h>
#include <utils/chiches.h>

// SEMAFOROS
extern pthread_mutex_t mutex_new;
extern pthread_mutex_t mutex_ready;
extern pthread_mutex_t mutex_exit;
extern pthread_mutex_t mutex_running;
extern pthread_mutex_t mutex_block;
extern pthread_mutex_t mutex_susp_block;
extern pthread_mutex_t mutex_susp_ready;
//extern pthread_mutex_t mutex_io;           // Protege el acceso a la lista de dispositivos IO
extern sem_t hay_espacio_en_memoria;
extern sem_t habilitar_corto_plazo;

// =================== AGREGAR A COLAS =========================
void agregar_a_new(t_pcb* pcb);
void agregar_a_ready(t_pcb* pcb);
void agregar_a_running(t_pcb* pcb);
void agregar_a_exit(t_pcb* pcb);
void agregar_a_block(t_pcb* pcb);
void agregar_a_susp_block(t_pcb* pcb);
void agregar_a_susp_ready(t_pcb* pcb);

void inicializar_cola(t_queue* cola_origen, pthread_mutex_t* mutex, status_cod origen, status_cod destino);
void intentar_inicializar_procesos_pendientes();
void inicializar_listasDeEstados();

// =================== SEMAFOROS =========================
void inicializar_semaforos();
void liberar_semaforos();

#endif