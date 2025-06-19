#include "instrucciones.h"

// Parser simple que separa la instruccion en part
// Funcion auxiliar de parseo (parsing function, convierte un string en una estructura usable)
// EJEMPLO: "WRITE 10 HOLA" --> "WRITE" "10" "HOLA"

t_parsed_instruccion parse_instruccion(const char* instruccion_str) {
    t_parsed_instruccion resultado;

    char* copia = strdup(instruccion_str); // copiamos porque strtok modifica
    
    resultado.opcode = strtok(copia, " ");
    resultado.arg1 = strtok(NULL, " ");
    resultado.arg2 = strtok(NULL, "");

    // Asegurar que los parametros NO sean NULL
    if(!resultado.arg1) resultado.arg1 = strdup("-");
    if(!resultado.arg2) resultado.arg2 = strdup("-");

    return resultado;
} 

void enviar_instruccion(int socket_cliente, char *instruccion, t_list *parametros) {
    
    t_tipo_instruccion tipo_instruccion = obtener_tipo_instruccion(instruccion);
    t_paquete *paquete = crear_paquete(tipo_instruccion);//
    //agregar_a_paquete_lista_string(paquete, parametros); // no tenemos essa funcion aun // fijar si la necesitamos
    agregar_a_paquete(paquete, instruccion, strlen(instruccion) + 1); // +1 para el \0
    enviar_paquete(paquete, socket_cliente); // enviar el paquete
    eliminar_paquete(paquete); // liberar el paquete
}

t_instruccion* recibir_instruccion(int socket_cliente){
    
    int tamanio;
    t_instruccion *instruccion = malloc(sizeof(t_instruccion));
    recv(socket_cliente,&tamanio,sizeof(int),MSG_WAITALL);

    void* buffer = recibir_buffer(&tamanio,socket_cliente);

    memcpy(instruccion, buffer, sizeof(t_instruccion));// copia los bytes al espacio de memoria apuntado por instruccion
    
    free(buffer);

    return instruccion;
}

void solicitar_instruccion(int socket_server, int PID, int program_counter) {
    t_paquete *paquete = crear_paquete(SOLICITAR_INSTRUCCION);
    agregar_a_paquete(paquete, &PID, sizeof(int));
    agregar_a_paquete(paquete, &program_counter, sizeof(int));
    enviar_paquete(paquete, socket_server);
    eliminar_paquete(paquete);
}

t_tipo_instruccion obtener_tipo_instruccion(char *instruccion) {
    
    if(strcmp(instruccion, "NOOP") == 0) {
        sleep(1); 
    } else if(strcmp(instruccion, "WRITE") == 0){
        return WRITE;
    } else if(strcmp(instruccion, "READ") == 0){
        return READ;
    } else if(strcmp(instruccion, "GOTO") == 0){
        return GOTO;
    } else if(strcmp(instruccion, "IO") == 0){
        return IO;
    } else if(strcmp(instruccion, "INIT_PROC") == 0){
        return INIT_PROC;
    } else if(strcmp(instruccion, "DUMP_MEMORY") == 0){
        return DUMP_MEMORY;
    }else if(strcmp(instruccion, "EXIT") == 0) {
        return EXIT;
    }    
    return 0;
}