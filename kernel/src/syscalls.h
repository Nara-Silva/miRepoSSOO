#ifndef SYSCALLS_H
#define SYSCALLS_H

#include <pcb.h>
#include <utils/protocolo.h>
#include <io.h>

char* extraer_instruccion(void* buffer, int* desplazamiento);
void enviar_a_ejecutar_a_cpu(t_pcb* pcb);
void manejarSyscall(t_pcb* pcb, char* instruccion_str);
void syscall_io(t_pcb* pcb, char* dispositivo, int tiempo);
void syscall_init_proc(char* nombre, int size);
void syscall_dump_memory(t_pcb* pcb);
void syscall_exit(t_pcb* pcb);

#endif