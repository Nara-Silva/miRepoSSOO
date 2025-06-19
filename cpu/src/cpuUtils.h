#ifndef UTILS_H_
#define UTILS_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <commons/log.h>
#include <utils/chiches.h>
#include <pthread.h>
#include <commons/config.h>
#include <utils/protocolo.h>


#define TRUE 1

typedef struct {
	char* modulo;
    char* ip_memoria; 
	char* puerto_memoria;
	char* ip_kernel;
	char* puerto_kernel_dispatch;
	char* puerto_kernel_interrupt;
	char* entradas_tlb;
	char* reemplazo_tlb;
	char* entradas_cache;
	char* reemplazo_cache;
	char* retardo_cache;
	char* log_level;
} t_config_cpu;

extern t_log* loggerCpu; 
extern t_config* config; 
void liberar_conexion(int fd);
void cargar_config ();
void inicializar_config(void);
void crear_logger();
void* iniciar_conexion_kernel(void* arg);
void* iniciar_conexion_memoria(void* arg);
t_log* iniciar_logger(char* nombreArhcivoLog, char* nombreLog, bool seMuestraEnConsola, t_log_level nivelDetalle);

#endif /* UTILS_H_ */