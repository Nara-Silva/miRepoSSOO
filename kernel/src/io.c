#include "io.h"

// =====================IO========================
//DISPOSITIVO IO

void* iniciar_servidor_io(void* arg) {
    int fd_sv = crear_servidor(config_struct->puerto_escucha_io);
    log_info(loggerKernel, "Servidor de IO iniciado en puerto %s", config_struct->puerto_escucha_io);
    
    while (true) {
        int fd_conexion = esperar_cliente(fd_sv, "IO", loggerKernel);
        if (fd_conexion > 0) {
            int operacion = recibir_operacion(fd_conexion);

            if (operacion == HANDSHAKE_IO) {
                int size;
                void* buffer = recibir_buffer(&size, fd_conexion);
                char* dispositivoIo = strdup(buffer);
                free(buffer);
                log_info(loggerKernel, "Conexión de IO: %s", dispositivoIo);
                
                if (!(dictionary_has_key(diccionarioDispositivosIO, dispositivoIo))) {
                    registrarNuevoIO(dispositivoIo, fd_conexion);
                }
                
                dispIO* disp = dictionary_get(diccionarioDispositivosIO, dispositivoIo);
                pthread_t hilo;
                pthread_create(&hilo, NULL, escucharRtasIo, (void*)disp);
                pthread_detach(hilo);
            }
        }
    }
    
    close(fd_sv);
    return NULL;
}

void* escucharRtasIo(void* arg) {
    dispIO* dispositivo = (dispIO*)arg;
    int fd_conexion = dispositivo->socket;
    
    while (true) {
        int cod_op = recibir_operacion(fd_conexion);
        
        if (cod_op == FIN_IO) {
            int pid;
            recv(fd_conexion, &pid, sizeof(int), MSG_WAITALL);
            
            t_pcb* pcb = buscar_pcb_por_pid(pid);
            if (pcb) {
                log_info(loggerKernel, "## (%d) finalizó IO y pasa a READY", pcb->pid);
                
                // Verificar si hay más procesos esperando
                if (!queue_is_empty(dispositivo->cola_bloqueadosporIO)) {
                    pthread_mutex_lock(&dispositivo->mutex_io);
                    t_pcb* pcb_desbloqueado = queue_pop(dispositivo->cola_bloqueadosporIO);
                    int* tiempo = queue_pop(dispositivo->tiempo_bloqueadosporIO);
                    int tiempo_real = *tiempo;
                    pthread_mutex_unlock(&dispositivo->mutex_io);
                    
                    solicitar_io(dispositivo->socket, pcb_desbloqueado, tiempo_real);
                    free(tiempo);
                } else {
                    dispositivo->libre = true;
                    dispositivo->pid_actual = -1;
                }
                
                // Cambiar estado del proceso que terminó IO
                if (pcb->estado == BLOCKED) {
                    cambiar_estado(pcb, BLOCKED, READY);
                    agregar_a_ready(pcb);
                } else if (pcb->estado == SUSP_BLOCKED) {
                    cambiar_estado(pcb, SUSP_BLOCKED, SUSP_READY);
                    agregar_a_susp_ready(pcb);
                }
            }
        } else if (cod_op == DESCONEXION_IO) {
            // Dispositivo se desconectó
            log_warning(loggerKernel, "Dispositivo IO %s se desconectó", dispositivo->nombre);
            
            // Si hay un proceso ejecutando en este dispositivo, enviarlo a EXIT
            if (dispositivo->pid_actual != -1) {
                t_pcb* pcb = buscar_pcb_por_pid(dispositivo->pid_actual);
                if (pcb) {
                    cambiar_estado(pcb, BLOCKED, SALIDA);
                    finalizar_proceso(pcb);
                }
            }
            
            // Limpiar cola de espera
            pthread_mutex_lock(&dispositivo->mutex_io);
            while (!queue_is_empty(dispositivo->cola_bloqueadosporIO)) {
                t_pcb* pcb = queue_pop(dispositivo->cola_bloqueadosporIO);
                int* tiempo = queue_pop(dispositivo->tiempo_bloqueadosporIO);
                free(tiempo);
                
                cambiar_estado(pcb, BLOCKED, SALIDA);
                finalizar_proceso(pcb);
            }
            pthread_mutex_unlock(&dispositivo->mutex_io);
            
            break; // Salir del bucle
        }
    }
    
    close(fd_conexion);
    return NULL;
}

//maneja la cola de procesos esperando cada IO
void agregarProcesosIo(dispIO* dispositivo, t_pcb* pcb, int tiempo) {
    if (dispositivo->libre) {
        dispositivo->libre = false;
        dispositivo->pid_actual = pcb->pid;
        solicitar_io(dispositivo->socket, pcb, tiempo);
    } else {
        cambiar_estado(pcb, EXEC, BLOCKED);
        agregar_a_block(pcb);
        
        int* tiempo_ptr = malloc(sizeof(int));
        *tiempo_ptr = tiempo;
        pthread_mutex_lock(&dispositivo->mutex_io);
        queue_push(dispositivo->tiempo_bloqueadosporIO, tiempo_ptr);
        queue_push(dispositivo->cola_bloqueadosporIO, pcb);
        pthread_mutex_unlock(&dispositivo->mutex_io);
    }
}

//Esta función se usa cuando el dispositivo está libre.
void solicitar_io(int fd_conexion_io, t_pcb* pcb, int tiempo) {
    log_info(loggerKernel, "## (%d) - Inicio de IO - Tiempo: %d", pcb->pid, tiempo);
    
    t_paquete* paquete = crear_paquete(SOLICITUD_IO);
    agregar_a_paquete(paquete, &(pcb->pid), sizeof(int));
    agregar_a_paquete(paquete, &tiempo, sizeof(int));
    enviar_paquete(paquete, fd_conexion_io);
    eliminar_paquete(paquete);
}

void registrarNuevoIO(char* nombre, int socket_fd) {
    dispIO* dispositivo = malloc(sizeof(dispIO));
    dispositivo->nombre = strdup(nombre);
    dispositivo->socket = socket_fd;
    dispositivo->libre = true;
    dispositivo->pid_actual = -1;
    pthread_mutex_init(&dispositivo->mutex_io, NULL);
    dispositivo->cola_bloqueadosporIO = queue_create();
    dispositivo->tiempo_bloqueadosporIO = queue_create();
    
    pthread_mutex_lock(&mutex_diccionario_io);
    dictionary_put(diccionarioDispositivosIO, nombre, dispositivo);
    pthread_mutex_unlock(&mutex_diccionario_io);
    
    log_info(loggerKernel, "Nuevo dispositivo IO registrado: %s", nombre);
}