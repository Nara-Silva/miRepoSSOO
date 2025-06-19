#include "planificacion.h"

// Variables globales para planificación
static int pid_global = 0;
static t_list* cpus_conectadas = NULL;
static t_dictionary* cpus_sockets = NULL;

// =================== PLANIFICACION =========================

// =================== LARGO PLAZO =========================
void* planificador_largo_plazo(void* arg) {
    char* leido;
    log_info(loggerKernel, "Presione ENTER para iniciar");
    leido = readline("");
    if (leido != NULL) {
        if (strcmp(leido, "") == 0) {
            log_info(loggerKernel, "Planificador de Largo plazo iniciado");
            
            // Procesar cola NEW según el algoritmo configurado
            while (true) {
                pthread_mutex_lock(&mutex_new);
                if (!queue_is_empty(cola_new)) {
                    t_pcb* proceso = queue_peek(cola_new);
                    pthread_mutex_unlock(&mutex_new);
                    
                    if (strcmp(config_struct->algoritmo_ingreso_a_ready, "FIFO") == 0) {
                        planificador_fifo(proceso);
                    } else if (strcmp(config_struct->algoritmo_ingreso_a_ready, "PMCP") == 0) {
                        planificador_pmcp(proceso);
                    }
                } else {
                    pthread_mutex_unlock(&mutex_new);
                    usleep(100000); // 100ms
                }
            }
        }
        free(leido);
    }
    return NULL;
}

void planificador_fifo(t_pcb* nuevo_proceso) {
    log_info(loggerKernel, "Proceso %d con FIFO", nuevo_proceso->pid);
    
    pthread_mutex_lock(&mutex_new);
    if (queue_is_empty(cola_new)) {
        pthread_mutex_unlock(&mutex_new);
        
        bool ok = solicitar_inicializacion_memoria(nuevo_proceso);
        if (ok) {
            cambiar_estado(nuevo_proceso, NEW, READY);
            agregar_a_ready(nuevo_proceso);
            sem_post(&habilitar_corto_plazo);
        } else {
            log_info(loggerKernel, "No hay memoria. Proceso %d queda esperando en NEW", nuevo_proceso->pid);
            agregar_a_new(nuevo_proceso);
            sem_wait(&hay_espacio_en_memoria);
        }
    } else {
        queue_push(cola_new, nuevo_proceso);
        pthread_mutex_unlock(&mutex_new);
        log_info(loggerKernel, "## (%d) No pudo inicializarse. Permanece en %s", nuevo_proceso->pid, "NEW");
    }
}

void planificador_pmcp(t_pcb* nuevo_proceso) {
    bool ok = solicitar_inicializacion_memoria(nuevo_proceso);
    if (ok) {
        pthread_mutex_lock(&mutex_new);
        if (queue_is_empty(cola_new) || comparar_por_size(nuevo_proceso, queue_peek(cola_new))) {
            pthread_mutex_unlock(&mutex_new);
            cambiar_estado(nuevo_proceso, NEW, READY);
            agregar_a_ready(nuevo_proceso);
            sem_post(&habilitar_corto_plazo);
        } else {
            queue_push(cola_new, nuevo_proceso);
            pthread_mutex_unlock(&mutex_new);
            log_info(loggerKernel, "## (%d) No pudo inicializarse. Permanece en %s", nuevo_proceso->pid, "NEW");
        }
    } else {
        agregar_a_new(nuevo_proceso);
        pthread_mutex_lock(&mutex_new);
        list_sort(cola_new->elements, comparar_por_size);
        pthread_mutex_unlock(&mutex_new);
    }
}

void planificador_sfj_sin_desalojo () {
    
}

void planificador_sfj_con_desalojo () {

}

// =================== CORTO PLAZO =========================
void* planificador_corto_plazo(void* arg) {
    while (true) {
        sem_wait(&habilitar_corto_plazo);
        
        pthread_mutex_lock(&mutex_ready);
        if (!queue_is_empty(cola_ready)) {
            t_pcb* proceso = queue_pop(cola_ready);
            pthread_mutex_unlock(&mutex_ready);
            
            cambiar_estado(proceso, READY, EXEC);
            agregar_a_running(proceso);
            
            // Enviar a CPU disponible
            enviar_a_ejecutar_a_cpu(proceso);
        } else {
            pthread_mutex_unlock(&mutex_ready);
        }
    }
    return NULL;
}

void planificador_sjf_sin_desalojo() {
    // Implementar SJF sin desalojo
    pthread_mutex_lock(&mutex_ready);
    if (!queue_is_empty(cola_ready)) {
        // Ordenar por tiempo estimado de ejecución
        list_sort(cola_ready->elements, comparar_por_tiempo_estimado);
        t_pcb* proceso = queue_pop(cola_ready);
        pthread_mutex_unlock(&mutex_ready);
        
        cambiar_estado(proceso, READY, EXEC);
        agregar_a_running(proceso);
        enviar_a_ejecutar_a_cpu(proceso);
    } else {
        pthread_mutex_unlock(&mutex_ready);
    }
}

void planificador_sjf_con_desalojo() {
    // Implementar SJF con desalojo (SRT)
    pthread_mutex_lock(&mutex_ready);
    if (!queue_is_empty(cola_ready)) {
        t_pcb* nuevo_proceso = queue_peek(cola_ready);
        
        // Verificar si hay procesos en ejecución con tiempo mayor
        pthread_mutex_lock(&mutex_running);
        if (!queue_is_empty(cola_exe)) {
            t_pcb* proceso_en_ejecucion = queue_peek(cola_exe);
            if (nuevo_proceso->tiempo_estimado < proceso_en_ejecucion->tiempo_restante) {
                // Desalojar proceso en ejecución
                queue_pop(cola_exe);
                pthread_mutex_unlock(&mutex_running);
                
                cambiar_estado(proceso_en_ejecucion, EXEC, READY);
                agregar_a_ready(proceso_en_ejecucion);
                
                // Enviar interrupción a CPU
                enviar_interrupcion_cpu(conexion_cpu, proceso_en_ejecucion->pid);
            }
        }
        pthread_mutex_unlock(&mutex_running);
        
        t_pcb* proceso = queue_pop(cola_ready);
        pthread_mutex_unlock(&mutex_ready);
        
        cambiar_estado(proceso, READY, EXEC);
        agregar_a_running(proceso);
        enviar_a_ejecutar_a_cpu(proceso);
    } else {
        pthread_mutex_unlock(&mutex_ready);
    }
}

// =================== MEDIANO PLAZO =========================
void* planificador_mediano_plazo(void* arg) {
    while (true) {
        // Verificar procesos bloqueados por tiempo
        pthread_mutex_lock(&mutex_block);
        t_list* procesos_a_suspender = list_create();
        
        for (int i = 0; i < list_size(cola_blocked->elements); i++) {
            t_pcb* proceso = list_get(cola_blocked->elements, i);
            if (proceso->tiempo_bloqueado >= atoi(config_struct->tiempo_suspension)) {
                list_add(procesos_a_suspender, proceso);
            }
        }
        pthread_mutex_unlock(&mutex_block);
        
        // Suspender procesos que excedieron el tiempo
        for (int i = 0; i < list_size(procesos_a_suspender); i++) {
            t_pcb* proceso = list_get(procesos_a_suspender, i);
            cambiar_estado(proceso, BLOCKED, SUSP_BLOCKED);
            agregar_a_susp_block(proceso);
            
            // Informar a memoria que mueva a SWAP
            enviar_suspension_memoria(proceso);
        }
        
        list_destroy(procesos_a_suspender);
        usleep(1000000); // 1 segundo
    }
    return NULL;
}

// =================== FUNCIONES AUXILIARES =========================
bool comparar_por_size(void* a, void* b) {
    t_pcb* pcb_a = (t_pcb*)a;
    t_pcb* pcb_b = (t_pcb*)b;
    return pcb_a->size < pcb_b->size;
}

bool comparar_por_tiempo_estimado(void* a, void* b) {
    t_pcb* pcb_a = (t_pcb*)a;
    t_pcb* pcb_b = (t_pcb*)b;
    return pcb_a->tiempo_estimado < pcb_b->tiempo_estimado;
}

void enviar_suspension_memoria(t_pcb* proceso) {
    int conexion_memoria = crear_conexion(config_struct->ip_memoria, config_struct->puerto_memoria);
    generar_handshake(conexion_memoria, loggerKernel);
    
    t_paquete* paquete = crear_paquete(SUSPENDER_PROCESO);
    agregar_a_paquete(paquete, &(proceso->pid), sizeof(int));
    enviar_paquete(paquete, conexion_memoria);
    eliminar_paquete(paquete);
    
    close(conexion_memoria);
}