#include "colas.h"

pthread_mutex_t mutex_new;
pthread_mutex_t mutex_ready;
pthread_mutex_t mutex_exit;
pthread_mutex_t mutex_running;
pthread_mutex_t mutex_block;
pthread_mutex_t mutex_susp_block;
pthread_mutex_t mutex_susp_ready;

// =================== INICIALIZACION =========================
// Creacion de colas con los diferentes estados
void inicializar_listasDeEstados(){
    cola_new = queue_create();
    cola_ready = queue_create();
    cola_blocked = queue_create();
    cola_exe = queue_create();
    cola_exit = queue_create();
    cola_susp_ready = queue_create();
    cola_susp_blocked = queue_create();
}

// =================== SEMAFOROS =========================
void inicializar_semaforos() {
    // Semaforos mutex para colas de Estados
    pthread_mutex_init(&mutex_new, NULL);
    pthread_mutex_init(&mutex_exit, NULL);
    pthread_mutex_init(&mutex_ready, NULL);  
    pthread_mutex_init(&mutex_running, NULL);
    pthread_mutex_init(&mutex_block, NULL);
    pthread_mutex_init(&mutex_susp_block, NULL);
    pthread_mutex_init(&mutex_susp_ready, NULL);
    // Semaforo mutex para IO
    //pthread_mutex_init(&mutex_io, NULL);
    // Semaforos de sincro para planificacion
    sem_init(&hay_espacio_en_memoria, 0, 0); // inicializado en 0
    sem_init(&habilitar_corto_plazo, 0, 0);
}

void liberar_semaforos() {
    pthread_mutex_destroy(&mutex_new);
    pthread_mutex_destroy(&mutex_exit);
    pthread_mutex_destroy(&mutex_ready);
    pthread_mutex_destroy(&mutex_running);
    pthread_mutex_destroy(&mutex_block);
    pthread_mutex_destroy(&mutex_susp_block);
    pthread_mutex_destroy(&mutex_susp_ready);
    
    sem_destroy(&hay_espacio_en_memoria);
    sem_destroy(&habilitar_corto_plazo);
}

// =================== COLAS =========================

void agregar_a_new(t_pcb* pcb) {
    pthread_mutex_lock(&mutex_new);
    queue_push(cola_new, pcb);
    pthread_mutex_unlock(&mutex_new);
    log_info(loggerKernel, "## PID: %d - Se agregó a NEW", pcb->pid);
}

void agregar_a_ready(t_pcb* pcb) {
    pthread_mutex_lock(&mutex_ready);
    queue_push(cola_ready, pcb);
    pthread_mutex_unlock(&mutex_ready);
    log_info(loggerKernel, "## PID: %d - Se agregó a READY", pcb->pid);
}

void agregar_a_exit(t_pcb* pcb) {
    pthread_mutex_lock(&mutex_exit);
    queue_push(cola_exit, pcb);
    pthread_mutex_unlock(&mutex_exit);
    log_info(loggerKernel, "## PID: %d - Se agregó a EXIT", pcb->pid);
}

void agregar_a_running(t_pcb* pcb) {
    pthread_mutex_lock(&mutex_running);
    queue_push(cola_exe, pcb);
    pthread_mutex_unlock(&mutex_running);
    log_info(loggerKernel, "## PID: %d - Se agregó a RUNNING", pcb->pid);
}

void agregar_a_block(t_pcb* pcb) {
    pthread_mutex_lock(&mutex_block);
    queue_push(cola_blocked, pcb);
    pthread_mutex_unlock(&mutex_block);
    log_info(loggerKernel, "## PID: %d - Se agregó a BLOCKED", pcb->pid);
}

void agregar_a_susp_block(t_pcb* pcb) {
    pthread_mutex_lock(&mutex_susp_block);
    queue_push(cola_susp_blocked, pcb);
    pthread_mutex_unlock(&mutex_susp_block);
    log_info(loggerKernel, "## PID: %d - Se agregó a SUSP BLOCKED", pcb->pid);
}

void agregar_a_susp_ready(t_pcb* pcb) {
    pthread_mutex_lock(&mutex_susp_ready);
    queue_push(cola_susp_ready, pcb);
    pthread_mutex_unlock(&mutex_susp_ready);
    log_info(loggerKernel, "## PID: %d - Se agregó a SUSP READY", pcb->pid);
}


void intentar_inicializar_procesos_pendientes() {
    inicializar_cola(cola_susp_ready, &mutex_susp_ready, SUSP_READY, READY);

    pthread_mutex_lock(&mutex_susp_ready);
    bool susp_vacia = queue_is_empty(cola_susp_ready);
    pthread_mutex_unlock(&mutex_susp_ready);

    if (susp_vacia) {
        inicializar_cola(cola_new, &mutex_new, NEW, READY);
    }
}

void inicializar_cola(t_queue* cola_origen, pthread_mutex_t* mutex, status_cod origen, status_cod destino) {
    while (true) {
        pthread_mutex_lock(mutex);
        if (queue_is_empty(cola_origen)) {
            pthread_mutex_unlock(mutex);
            break;
        }

        t_pcb* pcb = queue_peek(cola_origen); 
        pthread_mutex_unlock(mutex);

        if (!solicitar_inicializacion_memoria(pcb)) break;

        pthread_mutex_lock(mutex);
        queue_pop(cola_origen);
        pthread_mutex_unlock(mutex);

        cambiar_estado(pcb, origen, destino);

        pthread_mutex_lock(&mutex_ready);
        queue_push(cola_ready, pcb);
        pthread_mutex_unlock(&mutex_ready);
    }
}
