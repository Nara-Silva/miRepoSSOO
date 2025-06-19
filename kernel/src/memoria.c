#include "memoria.h"

bool solicitar_inicializacion_memoria(t_pcb* proceso) {
    int conexion_memoria = crear_conexion(config_struct->ip_memoria, config_struct->puerto_memoria);
    generar_handshake(conexion_memoria, loggerKernel);

    t_paquete* paquete = crear_paquete(INICIAR_PROCESO);
    agregar_a_paquete(paquete, &(proceso->pid), sizeof(int));
    agregar_a_paquete(paquete, &(proceso->size), sizeof(int));
    enviar_paquete(paquete, conexion_memoria);
    eliminar_paquete(paquete);

    // Esperamos la respuesta de memoria
    int32_t resultado;
    if (recv(conexion_memoria, &resultado, sizeof(int32_t), MSG_WAITALL) <= 0) {
        log_error(loggerKernel, "Error recibiendo respuesta de Memoria");
        close(conexion_memoria);
        return false;
    }
    close(conexion_memoria);
    
    if (resultado == 1) {
        // Memoria pudo inicializar el proceso
        return true;
    } else {
        // Memoria no tiene espacio suficiente
        return false;
    }
}

void enviar_finalizacion_memoria(t_pcb* pcb) {
    int conexion_memoria = crear_conexion(config_struct->ip_memoria, config_struct->puerto_memoria);
    generar_handshake(conexion_memoria, loggerKernel);

    t_paquete* paquete = crear_paquete(FINALIZAR_PROCESO);
    agregar_a_paquete(paquete, &(pcb->pid), sizeof(int));
    enviar_paquete(paquete, conexion_memoria);
    eliminar_paquete(paquete);

    // Esperar confirmación
    int32_t resultado;
    if (recv(conexion_memoria, &resultado, sizeof(int32_t), MSG_WAITALL) <= 0) {
        log_error(loggerKernel, "Error al recibir confirmación de finalización de proceso %d", pcb->pid);
    } else if (resultado == 1) {
        // Memoria confirmó la finalización
        sem_post(&hay_espacio_en_memoria); // Liberar espacio en memoria
    }
    
    close(conexion_memoria);
}

void enviar_suspension_memoria(t_pcb* proceso) {
    int conexion_memoria = crear_conexion(config_struct->ip_memoria, config_struct->puerto_memoria);
    generar_handshake(conexion_memoria, loggerKernel);

    t_paquete* paquete = crear_paquete(SUSPENDER_PROCESO);
    agregar_a_paquete(paquete, &(proceso->pid), sizeof(int));
    enviar_paquete(paquete, conexion_memoria);
    eliminar_paquete(paquete);

    // Esperar confirmación
    int32_t resultado;
    if (recv(conexion_memoria, &resultado, sizeof(int32_t), MSG_WAITALL) <= 0) {
        log_error(loggerKernel, "Error al recibir confirmación de suspensión de proceso %d", proceso->pid);
    } else if (resultado == 1) {
        // Memoria confirmó la suspensión
        sem_post(&hay_espacio_en_memoria); // Liberar espacio en memoria
    }
    
    close(conexion_memoria);
}

void enviar_desuspension_memoria(t_pcb* proceso) {
    int conexion_memoria = crear_conexion(config_struct->ip_memoria, config_struct->puerto_memoria);
    generar_handshake(conexion_memoria, loggerKernel);

    t_paquete* paquete = crear_paquete(DESUSPENDER_PROCESO);
    agregar_a_paquete(paquete, &(proceso->pid), sizeof(int));
    enviar_paquete(paquete, conexion_memoria);
    eliminar_paquete(paquete);

    // Esperar confirmación
    int32_t resultado;
    if (recv(conexion_memoria, &resultado, sizeof(int32_t), MSG_WAITALL) <= 0) {
        log_error(loggerKernel, "Error al recibir confirmación de desuspensión de proceso %d", proceso->pid);
    }
    
    close(conexion_memoria);
}