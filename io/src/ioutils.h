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
#include <commons/config.h>
#include <utils/protocolo.h>

#define TRUE 1
typedef struct {
    char* ip_kernel;
    char* puerto_kernel;
    char* modulo;
    char* log_level;
} t_config_io;

extern t_config_io* config_struct;
extern t_log* loggerIO;
extern t_config* config; 
void inicializar_config(void);
void cargar_config ();
void crear_logger ();
int iniciar_conexion_kernel();
void manejar_peticiones_io(int conexion_kernel);
void limpiar_recursos();
t_log* iniciar_logger(char* nombreArhcivoLog, char* nombreLog, bool seMuestraEnConsola, t_log_level nivelDetalle);

#endif /* UTILS_H_ */