#ifndef PCB_H
#define PCB_H

#include <commons/collections/dictionary.h>
#include "kernelUtils.h" // Para t_pcb, status_cod, etc.
#include "memoria.h"
#include "colas.h"
#include <commons/log.h>

extern t_dictionary* diccionarioDispositivosIO;
extern t_dictionary* diccionario_pcbs;

//================= DICCIONARIOS =================
void destruir_diccionarios();
void eliminar_pcb_diccionario(int pid);
t_pcb* buscar_pcb_por_pid(int pid);
void agregar_pcb_diccionario(t_pcb* pcb);
void inicializar_diccionarios();

// =================== PCB =========================
t_pcb* crear_pcb(int pid, int size);
void finalizar_proceso(t_pcb* pcb);
void generar_pid_unico(int* pid);
void log_estado_proceso(t_pcb* pcb, char* origen, char* destino);
void cambiar_estado(t_pcb* pcb, status_cod origen, status_cod destino);
void log_metricas_proceso(t_pcb* pcb);
void actualizar_metricas_estado (t_pcb* pcb, status_cod origen, status_cod destino);
const char* estado_to_str(status_cod estado);
void destruir_pcb(t_pcb* pcb);

//================== SEMAFOROS ==================
extern pthread_mutex_t mutex_pid;
extern pthread_mutex_t mutex_diccionario_io;
extern pthread_mutex_t mutex_diccionario_pcbs;
void inicializar_mutex_pid();

#endif