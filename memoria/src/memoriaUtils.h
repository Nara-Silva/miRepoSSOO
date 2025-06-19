#ifndef UTILS_H_
#define UTILS_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <pthread.h>
#include <commons/log.h>
#include <utils/chiches.h>
#include <commons/config.h>
#include <utils/protocolo.h>
#include <commons/collections/list.h>
#include <commons/collections/dictionary.h>
#include <time.h>

#define TRUE 1

typedef struct {
    char* modulo;
    char* puerto_escucha;
    char* tam_memoria;
    char* tam_pagina;
    char* entradas_por_tabla;
    char* cantidad_niveles;
    char* retardo_memoria;
    char* path_swapfile;
    char* retardo_swap;
    char* log_level;
    char* dump_path;
    char* path_instrucciones;
} t_config_memoria;

/*extern char* ip;
extern char* modulo;
extern char* puerto_kernel;
extern char* puerto_cpu;*/
extern t_log* loggerMemoria; 
extern t_config* config;

// Variables globales de memoria
extern void* memoria_principal;
extern t_dictionary* procesos_memoria;
extern t_list* entradas_swap;
extern FILE* archivo_swap;
extern int tamanio_memoria;
extern int tamanio_pagina;
extern int entradas_por_tabla;
extern int cantidad_niveles;
extern int retardo_memoria;
extern int retardo_swap;

// Funciones de configuración y logger
t_log* iniciar_logger(char* nombreArhcivoLog, char* nombreLog, bool seMuestraEnConsola, t_log_level nivelDetalle);
void liberar_conexion(int fd);
void cargar_config();
void inicializar_config(void);
void crear_logger();

// Funciones de servidor
void* iniciar_servidor_kernel();
void* iniciar_servidor_cpu();

// Funciones de gestión de procesos
t_proceso_memoria* crear_proceso_memoria(int pid, int tamanio, char* archivo_instrucciones);
void destruir_proceso_memoria(int pid);
void suspender_proceso(int pid);
void desuspender_proceso(int pid);
void finalizar_proceso(int pid);

// Funciones de tablas de páginas
int resolver_entrada_tabla(int pid, int tabla_actual, int entrada);
t_entrada_tabla*** crear_tablas_paginas(int niveles, int entradas_por_tabla);
void destruir_tablas_paginas(t_entrada_tabla*** tablas, int niveles, int entradas_por_tabla);

// Funciones de acceso a memoria
void leer_memoria(int pid, int direccion_fisica, int tamanio, void* buffer);
void escribir_memoria(int pid, int direccion_fisica, int tamanio, void* datos);
void leer_pagina_completa(int pid, int direccion_fisica, void* buffer);
void actualizar_pagina_completa(int pid, int direccion_fisica, void* datos);

// Funciones de instrucciones
char* obtener_instruccion(int pid, int pc);
void cargar_instrucciones_proceso(t_proceso_memoria* proceso);

// Funciones de SWAP
void inicializar_swap();
void cerrar_swap();
void escribir_pagina_swap(int pid, int pagina, void* datos, int tamanio);
void leer_pagina_swap(int pid, int pagina, void* buffer, int tamanio);
int obtener_posicion_swap(int pid, int pagina);
void liberar_entrada_swap(int pid, int pagina);

// Funciones de métricas
t_metricas_proceso* crear_metricas_proceso(int pid);
void incrementar_metrica_accesos_tablas(int pid);
void incrementar_metrica_instrucciones(int pid);
void incrementar_metrica_bajadas_swap(int pid);
void incrementar_metrica_subidas_memoria(int pid);
void incrementar_metrica_lecturas(int pid);
void incrementar_metrica_escrituras(int pid);
void log_metricas_proceso(int pid);

// Funciones de memory dump
void crear_memory_dump(int pid);

// Funciones de manejo de conexiones
void atender_solicitud_marco(int socket_cpu);
void atender_solicitud_instruccion(int socket_cpu);
void atender_solicitud_lectura(int socket_cpu);
void atender_solicitud_escritura(int socket_cpu);
void atender_solicitud_lectura_pagina(int socket_cpu);
void atender_solicitud_escritura_pagina(int socket_cpu);
void atender_solicitud_memory_dump(int socket_cpu);
void atender_solicitud_iniciar_proceso(int socket_kernel);
void atender_solicitud_suspender_proceso(int socket_kernel);
void atender_solicitud_desuspender_proceso(int socket_kernel);
void atender_solicitud_finalizar_proceso(int socket_kernel);

// Estructura para métricas por proceso
typedef struct {
    int pid;
    int accesos_tablas_paginas;
    int instrucciones_solicitadas;
    int bajadas_swap;
    int subidas_memoria_principal;
    int lecturas_memoria;
    int escrituras_memoria;
} t_metricas_proceso;

// Estructura para entrada de tabla de páginas
typedef struct {
    int marco;
    bool presente;
    bool modificada;
    int posicion_swap; // Posición en el archivo SWAP
} t_entrada_tabla;

// Estructura para proceso en memoria
typedef struct {
    int pid;
    int tamanio;
    void* espacio_memoria;
    t_entrada_tabla*** tablas_paginas; // Array de tablas por nivel
    t_metricas_proceso* metricas;
    char* archivo_instrucciones;
    char** instrucciones;
    int cantidad_instrucciones;
} t_proceso_memoria;

// Estructura para entrada en SWAP
typedef struct {
    int pid;
    int pagina;
    int posicion_inicio;
    int tamanio;
} t_entrada_swap;

#endif /* UTILS_H_ */