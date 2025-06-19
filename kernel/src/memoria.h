#ifndef MEMORIA_H
#define MEMORIA_H

#include "pcb.h"
#include "kernelUtils.h"
#include <stdbool.h>

bool solicitar_inicializacion_memoria(t_pcb* proceso);
void enviar_finalizacion_memoria(t_pcb* pcb);
void enviar_suspension_memoria(t_pcb* proceso);
void enviar_desuspension_memoria(t_pcb* proceso);

#endif