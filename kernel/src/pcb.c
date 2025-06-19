#include "pcb.h"
t_dictionary* diccionarioDispositivosIO = NULL;
t_dictionary* diccionario_pcbs = NULL;

pthread_mutex_t mutex_pid; // Declaración global
pthread_mutex_t mutex_diccionario_io;
pthread_mutex_t mutex_diccionario_pcbs;

void inicializar_mutex_pid() {
    pthread_mutex_init(&mutex_pid, NULL);
}

void inicializar_diccionarios(){
    diccionarioDispositivosIO = dictionary_create();
    diccionario_pcbs = dictionary_create();
    pthread_mutex_init(&mutex_diccionario_pcbs, NULL);
    pthread_mutex_init(&mutex_diccionario_io, NULL);
}

// Agrega un PCB al diccionario (clave: PID como string)
void agregar_pcb_diccionario(t_pcb* pcb) {
    char key[16];
    sprintf(key, "%d", pcb->pid); //escribe el valor del pid en la key
    pthread_mutex_lock(&mutex_diccionario_pcbs);
    dictionary_put(diccionario_pcbs, key, pcb);
    pthread_mutex_unlock(&mutex_diccionario_pcbs);
}

// Busca un PCB por PID, devuelve NULL si no existe
t_pcb* buscar_pcb_por_pid(int pid) {
    char key[16];
    sprintf(key, "%d", pid);
    pthread_mutex_lock(&mutex_diccionario_pcbs);
    t_pcb* pcb = (t_pcb*) dictionary_get(diccionario_pcbs, key);
    pthread_mutex_unlock(&mutex_diccionario_pcbs);
    return pcb;
}

// Elimina y destruye un PCB del diccionario
void eliminar_pcb_diccionario(int pid) {
    char key[16];
    sprintf(key, "%d", pid);
    pthread_mutex_lock(&mutex_diccionario_pcbs);
    dictionary_remove(diccionario_pcbs, key);
    pthread_mutex_unlock(&mutex_diccionario_pcbs);
//  if (pcb) destruir_pcb(pcb);
}

// Libera todo el diccionario y destruye los PCBs
void destruir_diccionarios() {
    dictionary_destroy(diccionario_pcbs);
    dictionary_destroy(diccionarioDispositivosIO);
}

// =================== PCB =========================
t_pcb* crear_pcb(int pid, int size) {
    t_pcb* new_pcb = malloc(sizeof(t_pcb));
    memset(new_pcb, 0, sizeof(t_pcb));
    new_pcb->pid = pid;
    new_pcb->size = size;
    new_pcb->estado = NEW;
    new_pcb->pc = 0;
    new_pcb->tiempo_inicio_estado = time(NULL);
    new_pcb->tiempo_estimado = atoi(config_struct->estimacion_inicial);
    new_pcb->tiempo_restante = atoi(config_struct->estimacion_inicial);
    new_pcb->tiempo_bloqueado = 0;
    new_pcb->archivo_instrucciones = NULL;
    
    // Inicializar métricas
    for (int i = 0; i < CANTIDAD_ESTADOS; i++) {
        new_pcb->metricas_estado[i] = 0;
        new_pcb->metricas_tiempo[i] = 0;
    }
    
    agregar_pcb_diccionario(new_pcb);
    return new_pcb;
}

void finalizar_proceso(t_pcb* pcb) {
    enviar_finalizacion_memoria(pcb);
    log_info(loggerKernel, "## (%d) - Finaliza el proceso", pcb->pid);
    log_metricas_proceso(pcb);
    eliminar_pcb_diccionario(pcb->pid);
    destruir_pcb(pcb);
    intentar_inicializar_procesos_pendientes();
}

void destruir_pcb(t_pcb* pcb) {
    if (pcb->archivo_instrucciones) {
        free(pcb->archivo_instrucciones);
    }
    free(pcb);
}

void generar_pid_unico(int* pid) {
    pthread_mutex_lock(&mutex_pid);
    ++(*pid);
    pthread_mutex_unlock(&mutex_pid);
}

void cambiar_estado(t_pcb* pcb, status_cod origen, status_cod destino) {
    log_info(loggerKernel, "## (%d) Pasa del estado %s al estado %s", pcb->pid, estado_to_str(origen), estado_to_str(destino));
    actualizar_metricas_estado(pcb, origen, destino);
    pcb->estado = destino;
    pcb->tiempo_inicio_estado = time(NULL);
}

void actualizar_metricas_estado(t_pcb* pcb, status_cod origen, status_cod destino) {
    // Actualizar conteo de estados
    pcb->metricas_estado[origen]++;
    
    // Calcular tiempo en el estado anterior
    time_t tiempo_actual = time(NULL);
    int tiempo_en_estado = (int)(tiempo_actual - pcb->tiempo_inicio_estado);
    pcb->metricas_tiempo[origen] += tiempo_en_estado;
    
    // Si el proceso se bloquea, iniciar contador de tiempo bloqueado
    if (destino == BLOCKED) {
        pcb->tiempo_bloqueado = 0;
    }
}

// ​Funciones de utilidad
const char* estado_to_str(status_cod estado) {
    switch (estado) {
        case NEW: return "NEW";
        case READY: return "READY";
        case EXEC: return "EXEC";
        case BLOCKED: return "BLOCKED";
        case SUSP_READY: return "SUSP_READY";
        case SUSP_BLOCKED: return "SUSP_BLOCKED";
        case SALIDA: return "EXIT";
        default: return "DESCONOCIDO";
    }
}

void log_metricas_proceso(t_pcb* pcb) {
    log_info(loggerKernel, "## (%d) - Métricas de estado: NEW (%d) (%dms), READY (%d) (%dms), EXEC (%d) (%dms), BLOCKED (%d) (%dms), SUSP_READY (%d) (%dms), SUSP_BLOCKED (%d) (%dms), EXIT (%d) (%dms)",
        pcb->pid,
        pcb->metricas_estado[NEW], pcb->metricas_tiempo[NEW],
        pcb->metricas_estado[READY], pcb->metricas_tiempo[READY],
        pcb->metricas_estado[EXEC], pcb->metricas_tiempo[EXEC],
        pcb->metricas_estado[BLOCKED], pcb->metricas_tiempo[BLOCKED],
        pcb->metricas_estado[SUSP_READY], pcb->metricas_tiempo[SUSP_READY],
        pcb->metricas_estado[SUSP_BLOCKED], pcb->metricas_tiempo[SUSP_BLOCKED],
        pcb->metricas_estado[SALIDA], pcb->metricas_tiempo[SALIDA]);
}