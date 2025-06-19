#ifndef CACHE_H
#define CACHE_H

#include <commons/collections/list.h>
#include <time.h>
#include <stdbool.h>
#include <cpuUtils.h>

typedef struct {
    int pid;
    int pagina;
    int marco;
    time_t ultimo_acceso;
    bool modificada;
} entrada_cache;

// Funciones de Cache
void inicializar_cache();
bool cache_habilitada();
bool cache_contiene_pagina(int pid, int pagina);
int obtener_marco_de_cache(int pid, int pagina);
void cache_agregar_pagina(int pid, int pagina, int marco);
void limpiar_cache();

extern t_list* CACHE;

#endif // CACHE_H
