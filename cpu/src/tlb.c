#include "tlb.h"
#include <string.h>
#include <stdlib.h>

t_list* TLB = NULL;

void inicializar_tlb() {
    TLB = list_create();
}

bool tlb_habilitada() {
    return atoi(config_struct->entradas_tlb) > 0;
}

bool tlb_contiene_pagina(int pid, int pagina) {
    for (int i = 0; i < list_size(TLB); i++) {
        entrada_tlb* e = list_get(TLB, i);
        if (e->pid == pid && e->pagina == pagina) {
            e->ultimo_acceso = time(NULL);
            return true;
        }
    }
    return false;
}

int obtener_marco_de_tlb(int pid, int pagina) {
    for (int i = 0; i < list_size(TLB); i++) {
        entrada_tlb* e = list_get(TLB, i);
        if (e->pid == pid && e->pagina == pagina) {
            return e->marco;
        }
    }
    return -1;
}

void tlb_agregar_entrada(int pid, int pagina, int marco) {
    entrada_tlb* nueva = malloc(sizeof(entrada_tlb));
    nueva->pid = pid;
    nueva->pagina = pagina;
    nueva->marco = marco;
    nueva->ultimo_acceso = time(NULL);

    int max_entradas = atoi(config_struct->entradas_tlb);
    if (list_size(TLB) >= max_entradas) {
        int index_victima = 0;

        if (strcmp(config_struct->reemplazo_tlb, "LRU") == 0) {
            time_t min_time = ((entrada_tlb*)list_get(TLB, 0))->ultimo_acceso;
            for (int i = 1; i < list_size(TLB); i++) {
                entrada_tlb* ent_tlb = list_get(TLB, i);
                if (ent_tlb->ultimo_acceso < min_time) {
                    min_time = ent_tlb->ultimo_acceso;
                    index_victima = i;
                }
            }
        }

        entrada_tlb* victima = list_remove(TLB, index_victima);
        free(victima);
    }

    list_add(TLB, nueva);
}

void limpiar_tlb() {
    list_clean_and_destroy_elements(TLB, free);
}
