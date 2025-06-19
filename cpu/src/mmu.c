#include "mmu.h"
#include "tlb.h"
#include "cache.h"
#include <cpuUtils.h>

// Variables globales para MMU (deberían venir de configuración)
int tamanio_pagina = 64; // Valor por defecto
int niveles_paginacion = 2; // Valor por defecto
int entradas_por_tabla = 4; // Valor por defecto

int traducir_direccion_logica_a_fisica(int pid, int direccion_logica) {
    int nro_pagina = direccion_logica / tamanio_pagina;
    int desplazamiento = direccion_logica % tamanio_pagina;

    int marco = -1;

    // -- Primero: verificar Cache / TLB ---
    // 1. Cache Hit
    if (cache_habilitada() && cache_contiene_pagina(pid, nro_pagina)) {
        log_info(loggerCpu, "PID: %d - Cache Hit - Pagina: %d", pid, nro_pagina);
        marco = obtener_marco_de_cache(pid, nro_pagina);
    }
    // 2. TLB Hit
    else if (tlb_habilitada() && tlb_contiene_pagina(pid, nro_pagina)) {
        log_info(loggerCpu, "PID: %d - TLB HIT - Pagina: %d", pid, nro_pagina);
        marco = obtener_marco_de_tlb(pid, nro_pagina);
    } 
    // 3. TLB Miss → pedir a Memoria
    else {
        log_info(loggerCpu, "PID: %d - TLB MISS - Pagina: %d", pid, nro_pagina);
    
        int tabla_actual = 0;
        // Se recorre la jerarquía de tablas de páginas nivel por nivel.
        for(int nivel = 1; nivel <= niveles_paginacion; nivel++) {
            int entrada = entrada_nivel(nro_pagina, nivel, niveles_paginacion, entradas_por_tabla);
            tabla_actual = solicitar_tabla_o_marco(config_struct->puerto_memoria, pid, tabla_actual, entrada);
        }

        marco = tabla_actual;
    }

    // Si se tuvo que ir a memoria (TLB miss), 
    // se almacena la traducción obtenida en TLB y Cache

    if (tlb_habilitada()) {
        tlb_agregar_entrada(pid, nro_pagina, marco);
    }

    if (cache_habilitada()) {
        cache_agregar_pagina(pid, nro_pagina, marco);
    }
    
    log_info(loggerCpu, "PID: %d - OBTENER MARCO - Página: %d - Marco: %d", pid, nro_pagina, marco);

    return marco * tamanio_pagina + desplazamiento;
}

int entrada_nivel(int nro_pagina, int nivel, int niveles_totales, int entradas_por_tabla) {
    int divisor = pow(entradas_por_tabla, niveles_totales - nivel);
    int resultado = (nro_pagina/divisor) % entradas_por_tabla;
    return resultado;
}

int solicitar_tabla_o_marco(int socket_memoria, int pid, int tabla_actual, int entrada) {
    op_code codigo = RESOLVER_ENTRADA_TABLA; 

    // Enviar código de operación
    send(socket_memoria, &codigo, sizeof(op_code), 0);

    // Enviar pid, tabla actual y entrada
    send(socket_memoria, &pid, sizeof(int), 0);
    send(socket_memoria, &tabla_actual, sizeof(int), 0);
    send(socket_memoria, &entrada, sizeof(int), 0);

    // Esperar respuesta
    int resultado = -1;
    int bytes = recv(socket_memoria, &resultado, sizeof(int), MSG_WAITALL);
    
    if (bytes <= 0) {
        log_error(loggerCpu, "Error al recibir respuesta desde Memoria al consultar tabla/marco");
        return -1;
    }

    return resultado;
}
