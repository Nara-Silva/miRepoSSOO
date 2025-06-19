#ifndef INSTRUCCIONES_H
#define INSTRUCCIONES_H

#include <commons/collections/list.h>
#include <utils/chiches.h>
#include <stdlib.h>
#include <string.h>
#include <utils/protocolo.h>
#include <math.h>


typedef enum {
/*============ CPU ============*/ 
    NOOP,
    WRITE,
    READ,
    GOTO,
/*============ SYSCALLS (Kernel) ============*/   
    IO,  
    INIT_PROC,
    DUMP_MEMORY,
    EXIT
} t_tipo_instruccion;

typedef struct {  
    t_tipo_instruccion tipo;
    int direccion;       // Para WRITE Y READ
    int tamanio;         // Para READ
    char datos[256];     // Para WRITE
    int nuevoPc_valor;   // Para GOTO
    char nombreDispositivo[32]; // Seguramente sea una struct // Para IO
    int tiempo;         // Para IO
    char archivoDeInstrucciones[256]; // Para 
    int tamanioProceso; // Para INIT_PROC 
} t_instruccion;

// Estructura para almacenar instruccion parseada
typedef struct {
    char* opcode;
    char* arg1;
    char* arg2;
} t_parsed_instruccion;

void enviar_instruccion(int socket_cliente, char *instruccion, t_list *parametros);
t_instruccion* recibir_instruccion(int socket_cliente);
void solicitar_instruccion(int socket_server, int PID, int program_counter);
t_tipo_instruccion obtener_tipo_instruccion(char *instruccion);
t_parsed_instruccion parse_instruccion(const char* instruccion_str);

#endif // INSTRUCCIONES_H