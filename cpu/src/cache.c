#include "cache.h"
#include <string.h>
#include <stdlib.h>

t_list* CACHE = NULL;

void inicializar_cache() {
    CACHE = list_create();
}

bool cache_habilitada() {
    return atoi(config_struct->entradas_cache) > 0;
}

bool cache_contiene_pagina(int pid, int pagina) {
    for (int i = 0; i < list_size(CACHE); i++) {
        entrada_cache* entr_cache = list_get(CACHE, i);
        if (entr_cache->pid == pid && entr_cache->pagina == pagina) {
            entr_cache->ultimo_acceso = time(NULL);
            return true;
        }
    }
    return false;
}

int obtener_marco_de_cache(int pid, int pagina) {
    for (int i = 0; i < list_size(CACHE); i++) {
        entrada_cache* entr_cache = list_get(CACHE, i);
        if (entr_cache->pid == pid && entr_cache->pagina == pagina) {
            return entr_cache->marco;
        }
    }
    return -1;
}

void cache_agregar_pagina(int pid, int pagina, int marco) {
    entrada_cache* nueva = malloc(sizeof(entrada_cache));
    nueva->pid = pid;
    nueva->pagina = pagina;
    nueva->marco = marco;
    nueva->ultimo_acceso = time(NULL);
    nueva->modificada = false;

    int max_entradas = atoi(config_struct->entradas_cache);
    if (list_size(CACHE) >= max_entradas) {
        int index_victima = 0;

        if (strcmp(config_struct->reemplazo_cache, "LRU") == 0) {
            time_t min_time = ((entrada_cache*)list_get(CACHE, 0))->ultimo_acceso;
            for (int i = 1; i < list_size(CACHE); i++) {
                entrada_cache* entr_cache = list_get(CACHE, i);
                if (entr_cache->ultimo_acceso < min_time) {
                    min_time = entr_cache->ultimo_acceso;
                    index_victima = i;
                }
            }
        }

        entrada_cache* victima = list_remove(CACHE, index_victima);
        free(victima);
    }

    list_add(CACHE, nueva);
}

void limpiar_cache() {
    list_clean_and_destroy_elements(CACHE, free);
}
