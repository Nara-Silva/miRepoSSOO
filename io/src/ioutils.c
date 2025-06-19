#include "ioutils.h"

t_log* loggerIO = NULL;
t_config_io* config_struct = NULL;
t_config* config = NULL;

void inicializar_config(void){
    config_struct = malloc(sizeof(t_config_io));
    config_struct->ip_kernel = NULL;
    config_struct->puerto_kernel = NULL;
    config_struct->modulo = NULL;
    config_struct->log_level = NULL;
}

int iniciar_conexion_kernel(){
    log_info(loggerIO, "Conectando al Kernel en %s:%s", config_struct->ip_kernel, config_struct->puerto_kernel);
    
    int conexion_kernel = crear_conexion(config_struct->ip_kernel, config_struct->puerto_kernel);
    if (conexion_kernel < 0) {
        log_error(loggerIO, "Error al crear conexión con el Kernel");
        return -1;
    }
    
    if (!generar_handshake(conexion_kernel, loggerIO)) {
        log_error(loggerIO, "Error en handshake con el Kernel");
        close(conexion_kernel);
        return -1;
    }
    
    log_info(loggerIO, "Conexión establecida exitosamente con el Kernel");
    return conexion_kernel;
}

t_log* iniciar_logger(char* nombreArhcivoLog, char* nombreLog, bool seMuestraEnConsola, t_log_level nivelDetalle)
{
	t_log* nuevo_logger;
	nuevo_logger = log_create(nombreArhcivoLog, nombreLog, seMuestraEnConsola, nivelDetalle);
    if (nuevo_logger == NULL) {
		printf("Error en el logger");
		exit(EXIT_FAILURE);
		}
	return nuevo_logger;
}

// Crea y configura el logger
void crear_logger() {
    // Determinar el nivel de log desde la configuración
    t_log_level nivel_log = LOG_LEVEL_INFO; // Por defecto
    
    if (config_struct && config_struct->log_level) {
        if (strcmp(config_struct->log_level, "TRACE") == 0) {
            nivel_log = LOG_LEVEL_TRACE;
        } else if (strcmp(config_struct->log_level, "DEBUG") == 0) {
            nivel_log = LOG_LEVEL_DEBUG;
        } else if (strcmp(config_struct->log_level, "INFO") == 0) {
            nivel_log = LOG_LEVEL_INFO;
        } else if (strcmp(config_struct->log_level, "WARNING") == 0) {
            nivel_log = LOG_LEVEL_WARNING;
        } else if (strcmp(config_struct->log_level, "ERROR") == 0) {
            nivel_log = LOG_LEVEL_ERROR;
        }
    }
    
    loggerIO = iniciar_logger("io.log", "IO", TRUE, nivel_log);
    log_info(loggerIO, "Logger IO inicializado con nivel: %s", config_struct->log_level ? config_struct->log_level : "INFO");
}

// Carga la configuración desde el archivo
void cargar_config() {
    config = config_create("io.config");
    config_struct->modulo = config_get_string_value(config, "MODULO");
    config_struct->ip_kernel = config_get_string_value(config, "IP_KERNEL");
    config_struct->puerto_kernel = config_get_string_value(config, "PUERTO_KERNEL");
    config_struct->log_level = config_get_string_value(config, "LOG_LEVEL");
}

//==============MANEJO PETICIONES==============

void manejar_peticiones_io(int conexion_kernel) {
    log_info(loggerIO, "Iniciando manejo de peticiones IO");
    
    while (true) {
        int cod_op = recibir_operacion(conexion_kernel);
        
        if (cod_op == SOLICITUD_IO) {
            // Recibir PID y tiempo de la petición
            int pid, tiempo;
            int bytes_pid = recv(conexion_kernel, &pid, sizeof(int), MSG_WAITALL);
            int bytes_tiempo = recv(conexion_kernel, &tiempo, sizeof(int), MSG_WAITALL);
            
            if (bytes_pid <= 0 || bytes_tiempo <= 0) {
                log_error(loggerIO, "Error al recibir datos de la petición IO");
                break;
            }
            
            // Log de inicio de IO (OBLIGATORIO)
            log_info(loggerIO, "## PID: %d - Inicio de IO - Tiempo: %d", pid, tiempo);
            
            // Simular la operación de IO con usleep
            usleep(tiempo * 1000);  // Convertir milisegundos a microsegundos
            
            // Log de finalización de IO (OBLIGATORIO)
            log_info(loggerIO, "## PID: %d - Fin de IO", pid);
            
            // Enviar confirmación de finalización al Kernel
            t_paquete* respuesta = crear_paquete(FIN_IO);
            agregar_a_paquete(respuesta, &pid, sizeof(int));
            enviar_paquete(respuesta, conexion_kernel);
            eliminar_paquete(respuesta);
            
        } else if (cod_op == HANDSHAKE) {
            // Handshake inicial, continuar
            log_info(loggerIO, "Handshake recibido del Kernel");
            continue;
            
        } else {
            log_error(loggerIO, "Operación desconocida recibida: %d. Cerrando IO...", cod_op);
            break;
        }
    }
    
    log_info(loggerIO, "Finalizando manejo de peticiones IO");
}

// Limpiar recursos al finalizar
void limpiar_recursos() {
    if (config_struct) {
        if (config_struct->ip_kernel) free(config_struct->ip_kernel);
        if (config_struct->puerto_kernel) free(config_struct->puerto_kernel);
        if (config_struct->modulo) free(config_struct->modulo);
        if (config_struct->log_level) free(config_struct->log_level);
        free(config_struct);
    }
    
    if (config) {
        config_destroy(config);
    }
    
    if (loggerIO) {
        log_destroy(loggerIO);
    }
}