#ifndef KERNELUTILS_H
#define KERNELUTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <commons/log.h>
#include <utils/chiches.h>
#include <utils/protocolo.h>
#include <pthread.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <commons/collections/dictionary.h> // Asegúrate de incluir esto
#include <semaphore.h>
#include <utils/instrucciones.h>

#define TRUE 1

typedef struct {
    char* ip_memoria;
    char* puerto_memoria;
    char* puerto_escucha_dispatch;
    char* puerto_escucha_interrupt;
    char* puerto_escucha_io;
    char* modulo;
    char* algoritmo_corto_plazo;
    char* algoritmo_ingreso_a_ready;
    char* alfa;
    char* estimacion_inicial;
    char* tiempo_suspension;
    char* log_level;
} t_config_kernel;

extern t_config_kernel* config_struct;

/*
typedef enum { //Enum para los algoritmos de ordenamiento 
    FIFO,
    SJF_CON_DESALOJO,
    SJF_SIN_DESALOJO,
    PMCP
} algoritmo;
*/

//CONEXION GLOBAL CON CPU
extern int conexion_cpu;
/*
// SEMAFOROS
extern pthread_mutex_t mutex_new;
extern pthread_mutex_t mutex_ready;
extern pthread_mutex_t mutex_exit;
extern pthread_mutex_t mutex_running;
extern pthread_mutex_t mutex_block;
extern pthread_mutex_t mutex_susp_block;
extern pthread_mutex_t mutex_susp_ready;
//extern pthread_mutex_t mutex_io;           // Protege el acceso a la lista de dispositivos IO
extern sem_t hay_espacio_en_memoria;
extern sem_t habilitar_corto_plazo;
*/

//LOG Y CONFIG

extern t_log* loggerKernel;
extern t_config* config;
/*
// =================== AGREGAR A COLAS =========================
void agregar_a_new(t_pcb* pcb);
void agregar_a_ready(t_pcb* pcb);
void agregar_a_running(t_pcb* pcb);
void agregar_a_exit(t_pcb* pcb);
void agregar_a_block(t_pcb* pcb);
void agregar_a_susp_block(t_pcb* pcb);
void agregar_a_susp_ready(t_pcb* pcb);
*/
// =================== MAIN Y BASIC =========================
void inicializar_config(void);
void inicializar_listasDeEstados();
void* iniciar_servidor_io(void* arg);
void* iniciar_servidor_dispatch(void* arg);
void* iniciar_servidor_interrupt(void* arg);
void* iniciar_conexion_memoria(void* arg);
void crear_logger();
void cargar_config();
void liberar_conexion(int fd);
t_log* iniciar_logger(char* nombreArhcivoLog, char* nombreLog, bool seMuestraEnConsola, t_log_level nivelDetalle);
/*

// =================== PLANIFICACION =========================
void planificador_fifo(t_pcb* nuevo_proceso);
void* planificador_largo_plazo(void* arg);
bool solicitar_inicializacion_memoria(t_pcb* proceso);
void enviar_finalizacion_memoria(t_pcb* pcb);
void enviar_a_ejecutar_a_cpu(t_pcb* pcb);
char* extraer_instruccion(void* buffer, int* desplazamiento);
void inicializar_cola(t_queue* cola_origen, pthread_mutex_t* mutex, status_cod origen, status_cod destino);
void intentar_inicializar_procesos_pendientes();
void* planificador_cortoplazo_fifo(void* arg);
void finalizar_proceso(t_pcb* proceso);
// =====================IO========================
*/
typedef struct {
    char* nombre;  // Nombre del dispositivo
    int socket;    // socket para hablar con io
    t_queue* cola_bloqueadosporIO;  // procesos esperando esta IO
    t_queue* tiempo_bloqueadosporIO; // tiempos de espera de los procesos
    bool libre;
    pthread_mutex_t mutex_io;
    int pid_actual;
} dispIO;

//extern t_dictionary* diccionarioDispositivosIO;
//extern t_dictionary* diccionario_pcbs;
/*
void manejarSyscall(t_pcb* pcb, char* instruccion_str);
char* extraer_instruccion(void* buffer, int* desplazamiento);
void agregarProcesosIo(dispIO* dispositivo, t_pcb* pcb, int tiempo);
void solicitar_io(int fd_conexion_io, t_pcb* pcb, int tiempo);
void syscall_io (t_pcb* pcb, char* dispositivo, int tiempo);
void* escucharRtasIo(void* arg);
dispIO* buscarPorNombre(char* nombre);
void inicializar_listaDispIo();
void registrarNuevoIO (char* nombre, int socket_fd);
*/
/*
// =================== PCB =========================
t_pcb* crear_pcb();
void generar_pid_unico(int* pid);
void log_estado_proceso(t_pcb* pcb, char* origen, char* destino);
void cambiar_estado(t_pcb* pcb, status_cod origen, status_cod destino);
void log_metricas_proceso(t_pcb* pcb);
void actualizar_metricas_estado (t_pcb* pcb, status_cod origen, status_cod destino);
const char* estado_to_str(status_cod estado);
void destruir_pcb(t_pcb* pcb);

// =================== SEMAFOROS =========================

void inicializar_semaforos();
void liberar_semaforos();
*/

// =================== PLANIFICACION =========================
void* planificador_largo_plazo(void* arg);
void* planificador_corto_plazo(void* arg);
void* planificador_mediano_plazo(void* arg);
void planificador_fifo(t_pcb* nuevo_proceso);
void planificador_pmcp(t_pcb* nuevo_proceso);
void planificador_sjf_sin_desalojo(void);
void planificador_sjf_con_desalojo(void);
bool solicitar_inicializacion_memoria(t_pcb* proceso);
void enviar_finalizacion_memoria(t_pcb* pcb);
void enviar_a_ejecutar_a_cpu(t_pcb* pcb);
void enviar_interrupcion_cpu(int socket_cpu, int pid);
char* extraer_instruccion(void* buffer, int* desplazamiento);
void inicializar_cola(t_queue* cola_origen, pthread_mutex_t* mutex, status_cod origen, status_cod destino);
void intentar_inicializar_procesos_pendientes();
void finalizar_proceso(t_pcb* proceso);
bool comparar_por_size(void* a, void* b);

// =====================IO========================
void* escucharRtasIo(void* arg);
void agregarProcesosIo(dispIO* dispositivo, t_pcb* pcb, int tiempo);
void solicitar_io(int fd_conexion_io, t_pcb* pcb, int tiempo);
void registrarNuevoIO(char* nombre, int socket_fd);
void manejarSyscall(t_pcb* pcb, char* instruccion_str);
void syscall_io(t_pcb* pcb, char* dispositivo, int tiempo);
void syscall_init_proc(char* nombre, int size);
void syscall_dump_memory(t_pcb* pcb);
void syscall_exit(t_pcb* pcb);

// =================== PCB =========================
t_pcb* crear_pcb(int pid, int size);
void generar_pid_unico(int* pid);
void log_estado_proceso(t_pcb* pcb, char* origen, char* destino);
void cambiar_estado(t_pcb* pcb, status_cod origen, status_cod destino);
void log_metricas_proceso(t_pcb* pcb);
void actualizar_metricas_estado(t_pcb* pcb, status_cod origen, status_cod destino);
const char* estado_to_str(status_cod estado);
void destruir_pcb(t_pcb* pcb);

// =================== SEMAFOROS =========================
void inicializar_semaforos();
void liberar_semaforos();

void* manejar_conexion_cpu_dispatch(void* arg);
void* manejar_conexion_cpu_interrupt(void* arg);

#endif

