#ifndef IO_H
#define IO_H

#include <kernelUtils.h> // Para dispIO, t_pcb
#include <pcb.h>
#include <utils/protocolo.h>
#include <string.h>

void* iniciar_servidor_io(void* arg);
void* escucharRtasIo(void* arg);
void agregarProcesosIo(dispIO* dispositivo, t_pcb* pcb, int tiempo);
void solicitar_io(int fd_conexion_io, t_pcb* pcb, int tiempo);
void registrarNuevoIO(char* nombre, int socket_fd);

#endif