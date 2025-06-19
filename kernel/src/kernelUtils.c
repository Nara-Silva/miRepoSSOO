#include "kernelUtils.h"

int conexion_cpu = -1;

//lista de estados que están en espera (hace falta?)
t_list* lista_exec;
t_log* loggerKernel= NULL; 
t_config* config = NULL;
t_config_kernel* config_struct;

// Inicializa la estructura de configuración
void inicializar_config(void){
    config_struct = malloc(sizeof(t_config_kernel)); //Reserva memoria
    config_struct->ip_memoria = NULL;
    config_struct->puerto_memoria = NULL;
    config_struct->puerto_escucha_dispatch = NULL;
    config_struct->puerto_escucha_interrupt = NULL;
    config_struct->puerto_escucha_io = NULL;
    config_struct->modulo = NULL;
    config_struct->algoritmo_corto_plazo = NULL;
    config_struct->algoritmo_ingreso_a_ready = NULL;
    config_struct->alfa = NULL;
    config_struct->estimacion_inicial = NULL;
    config_struct->tiempo_suspension = NULL;
    config_struct->log_level = NULL;
}

// Carga la configuración desde el archivo
void cargar_config() {
    config = config_create("kernel.config");
    config_struct->modulo = config_get_string_value(config, "MODULO");
    config_struct->ip_memoria = config_get_string_value(config, "IP_MEMORIA");
    config_struct->puerto_memoria = config_get_string_value(config, "PUERTO_MEMORIA");
    config_struct->puerto_escucha_dispatch = config_get_string_value(config, "PUERTO_ESCUCHA_DISPATCH");
    config_struct->puerto_escucha_interrupt = config_get_string_value(config, "PUERTO_ESCUCHA_INTERRUPT");
    config_struct->puerto_escucha_io = config_get_string_value(config, "PUERTO_ESCUCHA_IO");
    config_struct->algoritmo_corto_plazo = config_get_string_value(config, "ALGORITMO_CORTO_PLAZO");
    config_struct->algoritmo_ingreso_a_ready = config_get_string_value(config, "ALGORITMO_INGRESO_A_READY");
    config_struct->alfa = config_get_string_value(config, "ALFA");
    config_struct->estimacion_inicial = config_get_string_value(config, "ESTIMACION_INICIAL");
    config_struct->tiempo_suspension = config_get_string_value(config, "TIEMPO_SUSPENSION");
    config_struct->log_level = config_get_string_value(config, "LOG_LEVEL");
}

// Función para iniciar el logger
t_log* iniciar_logger(char* nombreArhcivoLog, char* nombreLog, bool seMuestraEnConsola, t_log_level nivelDetalle){
	t_log* nuevo_logger;
	nuevo_logger = log_create( nombreArhcivoLog, nombreLog, seMuestraEnConsola, nivelDetalle);
    if (nuevo_logger == NULL) {
		perror("Error en el logger"); // Maneja error si no se puede crear el logger
		exit(EXIT_FAILURE);
		}
	return nuevo_logger;
}

void* iniciar_conexion_memoria(void* arg){
    int conexion_memoria = crear_conexion(config_struct->ip_memoria, config_struct->puerto_memoria);
    generar_handshake(conexion_memoria, loggerKernel);
    return NULL;
}

// Inicia el servidor con CPU y acepta una conexion
void* iniciar_servidor_cpu (void* arg) {
    int fd_sv = crear_servidor(config_struct->puerto_escucha_dispatch); // Crea el servidor
    conexion_cpu = esperar_cliente(fd_sv, "CPU", loggerKernel); //Espera conexión
    close(fd_sv); //Cierra el servidor
    return NULL;
}

// Crea y configura el logger
void crear_logger () {
    loggerKernel=iniciar_logger("kernel.log","KERNEL",true, LOG_LEVEL_INFO);
}

// Inicia el servidor de dispatch para CPUs
void* iniciar_servidor_dispatch(void* arg) {
    int fd_sv = crear_servidor(config_struct->puerto_escucha_dispatch);
    log_info(loggerKernel, "Servidor de dispatch iniciado en puerto %s", config_struct->puerto_escucha_dispatch);
    
    while (true) {
        int fd_conexion = esperar_cliente(fd_sv, "CPU_DISPATCH", loggerKernel);
        if (fd_conexion > 0) {
            // Manejar conexión de CPU para dispatch
            pthread_t hilo_cpu;
            pthread_create(&hilo_cpu, NULL, manejar_conexion_cpu_dispatch, (void*)(long)fd_conexion);
            pthread_detach(hilo_cpu);
        }
    }
    
    close(fd_sv);
    return NULL;
}

// Inicia el servidor de interrupt para CPUs
void* iniciar_servidor_interrupt(void* arg) {
    int fd_sv = crear_servidor(config_struct->puerto_escucha_interrupt);
    log_info(loggerKernel, "Servidor de interrupt iniciado en puerto %s", config_struct->puerto_escucha_interrupt);
    
    while (true) {
        int fd_conexion = esperar_cliente(fd_sv, "CPU_INTERRUPT", loggerKernel);
        if (fd_conexion > 0) {
            // Manejar conexión de CPU para interrupt
            pthread_t hilo_cpu;
            pthread_create(&hilo_cpu, NULL, manejar_conexion_cpu_interrupt, (void*)(long)fd_conexion);
            pthread_detach(hilo_cpu);
        }
    }
    
    close(fd_sv);
    return NULL;
}

// Maneja conexión de CPU para dispatch
void* manejar_conexion_cpu_dispatch(void* arg) {
    int fd_conexion = (int)(long)arg;
    
    while (true) {
        int cod_op = recibir_operacion(fd_conexion);
        if (cod_op == DEVOLVER_PROCESO) {
            int pid, pc, motivo;
            recibir_pid_pc_motivo(fd_conexion, &pid, &pc, &motivo);
            
            t_pcb* pcb = buscar_pcb_por_pid(pid);
            if (pcb) {
                pcb->pc = pc;
                
                switch (motivo) {
                    case MOTIVO_SYSCALL:
                        // Recibir instrucción y manejar syscall
                        int size;
                        void* buffer = recibir_buffer(&size, fd_conexion);
                        char* instruccion_str = extraer_instruccion(buffer, &size);
                        free(buffer);
                        
                        log_info(loggerKernel, "## (%d) - Solicitó syscall: %s", pcb->pid, instruccion_str);
                        manejarSyscall(pcb, instruccion_str);
                        free(instruccion_str);
                        break;
                        
                    case MOTIVO_EXIT:
                        cambiar_estado(pcb, EXEC, SALIDA);
                        finalizar_proceso(pcb);
                        break;
                        
                    case MOTIVO_INTERRUPCION:
                        cambiar_estado(pcb, EXEC, READY);
                        agregar_a_ready(pcb);
                        break;
                        
                    default:
                        log_warning(loggerKernel, "PID %d - Motivo de devolución desconocido: %d", pid, motivo);
                        break;
                }
            }
        }
    }
    
    close(fd_conexion);
    return NULL;
}

// Maneja conexión de CPU para interrupt
void* manejar_conexion_cpu_interrupt(void* arg) {
    int fd_conexion = (int)(long)arg;
    
    // Solo escuchar interrupciones, no enviar nada
    while (true) {
        int cod_op = recibir_operacion(fd_conexion);
        if (cod_op == HANDSHAKE) {
            // CPU se conectó para recibir interrupciones
            log_info(loggerKernel, "CPU conectado para interrupciones");
        }
    }
    
    close(fd_conexion);
    return NULL;
}

// Envía interrupción a CPU específica
void enviar_interrupcion_cpu(int socket_cpu, int pid) {
    t_paquete* paquete = crear_paquete(INTERRUPCION);
    agregar_a_paquete(paquete, &pid, sizeof(int));
    enviar_paquete(paquete, socket_cpu);
    eliminar_paquete(paquete);
    log_info(loggerKernel, "## (%d) - Desalojado por algoritmo SJF/SRT", pid);
}

// Inicia el servidor con CPU y acepta una conexion
/*void* iniciar_servidor_cpu (void* arg) {
    int fd_sv = crear_servidor(config_struct->puerto_cpu); Crea el servidor
    while (1) {  // Mantiene el servidor activo
        int fd_conexion = esperar_cliente(fd_sv, "CPU", loggerKernel); Espera conexión
        
        // Manejamos la conexión de manera separada
        // y la cerramos cuando ya no se necesite
        close(fd_conexion); //Corta la conexión
    }
    close(fd_sv); //Cierra el servidor
}*/