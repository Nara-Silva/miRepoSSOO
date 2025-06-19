#include "cpuUtils.h"

t_log* loggerCpu = NULL; 
t_config* config = NULL;
t_config_cpu* config_struct;

// Inicializa la estructura de configuración
void inicializar_config(void) {
    config_struct = malloc(sizeof(t_config_cpu)); //Reserva memoria
    config_struct->modulo = NULL;
    config_struct->ip_memoria = NULL;
    config_struct->puerto_memoria = NULL;
    config_struct->ip_kernel = NULL;
    config_struct->puerto_kernel_dispatch = NULL;
    config_struct->puerto_kernel_interrupt = NULL;
    config_struct->entradas_tlb = NULL;
    config_struct->reemplazo_tlb = NULL;
    config_struct->entradas_cache = NULL;
    config_struct->reemplazo_cache = NULL;
    config_struct->retardo_cache = NULL;
    config_struct->log_level = NULL;
}

// Carga la configuración desde el archivo
void cargar_config () {
    config = config_create("cpu.config");
    config_struct->modulo = config_get_string_value (config, "MODULO");
    config_struct->ip_memoria = config_get_string_value (config, "IP_MEMORIA");
    config_struct->puerto_memoria = config_get_string_value(config, "PUERTO_MEMORIA");
    config_struct->ip_kernel = config_get_string_value(config, "IP_KERNEL");
    config_struct->puerto_kernel_dispatch = config_get_string_value(config, "PUERTO_KERNEL_DISPATCH");
    config_struct->puerto_kernel_interrupt = config_get_string_value(config, "PUERTO_KERNEL_INTERRUPT");
    config_struct->entradas_tlb = config_get_string_value(config, "ENTRADAS_TLB");
    config_struct->reemplazo_tlb = config_get_string_value(config, "REEMPLAZO_TLB");
    config_struct->entradas_cache = config_get_string_value(config, "ENTRADAS_CACHE");
    config_struct->reemplazo_cache = config_get_string_value(config, "REEMPLAZO_CACHE");
    config_struct->retardo_cache = config_get_string_value(config, "RETARDO_CACHE");
    config_struct->log_level = config_get_string_value(config, "LOG_LEVEL");
}

// Crea y configura el logger
void crear_logger () {
    loggerCpu=iniciar_logger("cpu.log","CPU", TRUE, LOG_LEVEL_INFO);
}

t_log* iniciar_logger(char* nombreAhrcivoLog, char* nombreLog, bool seMuestraEnConsola, t_log_level nivelDetalle) {
	t_log* nuevo_logger;
	nuevo_logger = log_create( nombreAhrcivoLog, nombreLog, seMuestraEnConsola, nivelDetalle);
    if (nuevo_logger == NULL) {
		perror("Error en el logger");
		exit(EXIT_FAILURE);
		}
	return nuevo_logger;
}

void* iniciar_conexion_memoria(void* arg){
    int conexion_memoria = crear_conexion(config_struct->ip_memoria, config_struct->puerto_memoria);
    int hand = generar_handshake(conexion_memoria, loggerCpu);
}

void* iniciar_conexion_kernel(void* arg){
    int conexion_kernel = crear_conexion(config_struct->ip_kernel, config_struct->puerto_kernel_dispatch);
    int hand = generar_handshake(conexion_kernel, loggerCpu);
}

void liberar_conexion(int fd) {
    close(fd);
}

