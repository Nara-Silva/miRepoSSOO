#ifndef TLB_H
#define TLB_H

#include <commons/collections/list.h>
#include <time.h>
#include <cpuUtils.h>

typedef struct {
    int pid;
    int pagina;
    int marco;
    time_t ultimo_acceso;
} entrada_tlb;

// Funciones de TLB
void inicializar_tlb();
bool tlb_habilitada();
bool tlb_contiene_pagina(int pid, int pagina);
int obtener_marco_de_tlb(int pid, int pagina);
void tlb_agregar_entrada(int pid, int pagina, int marco);
void limpiar_tlb();

extern t_list* TLB;

#endif // TLB_H
