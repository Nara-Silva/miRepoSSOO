#ifndef PROTOCOLO_H
#define PROTOCOLO_H

// chequear cuales realmente son necesarios 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <commons/log.h>
#include <stdint.h>
#include <utils/chiches.h>
typedef struct{
    int size;
    int offset;
    void* stream;
}t_buffer;

typedef enum { //podemos juntar todos los procesos 
    MENSAJE,
    PAQUETE,
    RECIBIR_PROCESO,
    INICIAR_PROCESO,
    FINALIZAR_PROCESO,
    SUSPENDER_PROCESO,
    DESUSPENDER_PROCESO,
    HANDSHAKE_IO, // para el caso de io
    SOLICITUD_IO,
    FIN_IO,
    DESCONEXION_IO,
    DEVOLVER_PROCESO,
    MOTIVO_SYSCALL,
    MOTIVO_EXIT,
    MOTIVO_INTERRUPCION,
    HANDSHAKE,
    INTERRUPCION,
    SOLICITAR_INSTRUCCION,
    SOLICITAR_MARCO = 50, // para que el mensaje no choque con el de handshake
    EXITO,
    RESOLVER_ENTRADA_TABLA,
    READ_MEMORIA,
    WRITE_MEMORIA,
    READ_PAGINA,
    WRITE_PAGINA,
    DUMP_MEMORY
} op_code;

typedef struct{
    int cod_op;
    t_buffer* buffer;
}t_paquete;

// Funciones de paquete y buffer
void destruir_buffer(t_buffer* buffer);
void recibir_pid_pc_motivo(int fd, int* pid, int* pc, int* motivo);
void enviar_pid_pc_motivo(int fd, int pid, int pc, int motivo);
t_paquete* crear_paquete(op_code operacion);
void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio);
void* serializar_paquete(t_paquete* paquete, int bytes);
void enviar_paquete(t_paquete* paquete, int socket_cliente);
void eliminar_paquete(t_paquete* paquete);

// Envío y recepción de mensajes
void enviar_mensaje(char* mensaje, int socket_cliente);
void recibir_mensaje(t_log* logger, int socket_cliente);

// Recepción de datos
void* recibir_buffer(int* size, int socket_cliente);
int recibir_operacion(int socket_cliente);

// ==============================

#endif
