#ifndef MMU_H
#define MMU_H

#include <math.h>
#include <cpuUtils.h>
#include <utils/protocolo.h>

// Variables globales para MMU
extern int tamanio_pagina;
extern int niveles_paginacion;
extern int entradas_por_tabla;

// Funciones de MMU
int traducir_direccion_logica_a_fisica(int pid, int direccion_logica);
int entrada_nivel(int nro_pagina, int nivel, int niveles_totales, int entradas_por_tabla);
int solicitar_tabla_o_marco(int socket_memoria, int pid, int tabla_actual, int entrada);

#endif // MMU_H
