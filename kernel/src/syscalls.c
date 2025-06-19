#include "syscalls.h"

static int pid = 0;

void enviar_a_ejecutar_a_cpu(t_pcb* pcb) {
    // Enviar PID y PC a CPU disponible
    enviar_pid_pc_motivo(conexion_cpu, pcb->pid, pcb->pc, MOTIVO_SYSCALL);
}

char* extraer_instruccion(void* buffer, int* desplazamiento) {
    int largo_instr;
    memcpy(&largo_instr, buffer + *desplazamiento, sizeof(int));
    *desplazamiento += sizeof(int);

    char* instruccion_str = malloc(largo_instr + 1);
    memcpy(instruccion_str, buffer + *desplazamiento, largo_instr);
    instruccion_str[largo_instr] = '\0';  // null-terminar

    *desplazamiento += largo_instr;
    
    return instruccion_str; // Devuelve la instrucción deserializada
}

/*============ SYSCALLS ===========*/
void manejarSyscall(t_pcb* pcb, char* instruccion_str) {
    t_parsed_instruccion inst = parse_instruccion(instruccion_str);
    
    log_info(loggerKernel, "## (%d) - Solicitó syscall: %s", pcb->pid, inst.opcode);
    
    if (strcmp(inst.opcode, "IO") == 0) {
        syscall_io(pcb, inst.arg1, atoi(inst.arg2));
    } else if (strcmp(inst.opcode, "INIT_PROC") == 0) {
        syscall_init_proc(inst.arg1, atoi(inst.arg2));
    } else if (strcmp(inst.opcode, "DUMP_MEMORY") == 0) {
        syscall_dump_memory(pcb);
    } else if (strcmp(inst.opcode, "EXIT") == 0) {
        syscall_exit(pcb);
    } else {
        log_warning(loggerKernel, "PID: %d - Syscall desconocida: %s", pcb->pid, inst.opcode);
    }
}

void syscall_io(t_pcb* pcb, char* dispositivo, int tiempo) {
    log_info(loggerKernel, "## (%d) - Bloqueado por IO: %s", pcb->pid, dispositivo);
    
    if (!dictionary_has_key(diccionarioDispositivosIO, dispositivo)) {
        log_error(loggerKernel, "Dispositivo IO %s no encontrado. Enviando proceso %d a EXIT", dispositivo, pcb->pid);
        cambiar_estado(pcb, EXEC, SALIDA);
        finalizar_proceso(pcb);
        return;
    }
    
    dispIO* dispositivoIO = dictionary_get(diccionarioDispositivosIO, dispositivo);
    agregarProcesosIo(dispositivoIO, pcb, tiempo);
}

void syscall_init_proc(char* nombre, int size) {
    generar_pid_unico(&pid);
    t_pcb* nuevo_proceso = crear_pcb(pid, size);
    nuevo_proceso->archivo_instrucciones = strdup(nombre);
    log_info(loggerKernel, "## (%d) - Se crea el proceso - Estado: NEW", nuevo_proceso->pid);
    agregar_a_new(nuevo_proceso);
}

void syscall_dump_memory(t_pcb* pcb) {
    // Crear conexión con memoria
    int conexion_memoria = crear_conexion(config_struct->ip_memoria, config_struct->puerto_memoria);
    generar_handshake(conexion_memoria, loggerKernel);
    
    // Enviar solicitud de dump
    t_paquete* paquete = crear_paquete(DUMP_MEMORY);
    agregar_a_paquete(paquete, &(pcb->pid), sizeof(int));
    enviar_paquete(paquete, conexion_memoria);
    eliminar_paquete(paquete);
    
    // Esperar confirmación
    int32_t resultado;
    if (recv(conexion_memoria, &resultado, sizeof(int32_t), MSG_WAITALL) <= 0) {
        log_error(loggerKernel, "Error al recibir confirmación de dump de memoria para proceso %d", pcb->pid);
        cambiar_estado(pcb, EXEC, SALIDA);
        finalizar_proceso(pcb);
    } else if (resultado == 0) {
        // Error en dump
        log_error(loggerKernel, "Error en dump de memoria para proceso %d", pcb->pid);
        cambiar_estado(pcb, EXEC, SALIDA);
        finalizar_proceso(pcb);
    } else {
        // Dump exitoso
        cambiar_estado(pcb, EXEC, READY);
        agregar_a_ready(pcb);
    }
    
    close(conexion_memoria);
}

void syscall_exit(t_pcb* pcb) {
    cambiar_estado(pcb, EXEC, SALIDA);
    finalizar_proceso(pcb);
}