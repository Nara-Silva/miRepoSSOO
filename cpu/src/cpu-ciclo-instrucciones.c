//////////////////// version 3 by reichel /////////////////////

/*
FLUJO COMPLETO DEBERÍA SER:
1. CPU recibe un proceso desde el Kernel ( o sea, kernel le envia a cpu un contexto de ejecucion: PID y PC)
2. CPU, con el PID y PC, pide instrucciones a la memoria
3. CPU recibe la instrucción
2. CPU inicia el ciclo de ejecución
5. CPU decodifica la instrucción
6. CPU ejecuta la instrucción (si son syscalls [IO(dispositivo, tiempo), INIT_PROC(archivoDeInstrucciones, tamanio), DUMP_MEMORY, EXIT] se envía al kernel para su realización)
7. CPU verifica si hay interrupciones (chequear si kernel envió una interrupcion al PID que se está ejecutando,
    si lo hizo, devuelve el PID y el PC actualizado al Kernel con "motivo" de la interrupción.
    Caso contrario, se descarta la interrupción)
8. Al finalizar el ciclo CPU incrementa el program counter en 1, si este no fue modificado por la instruccion GOTO
*/

#include "cpu-ciclo-instrucciones.h"

// Variables globlaes (ELIMINARLAS, tengo que pasarlas por parametro)
t_pcb *pcb_en_ejecucion;
//int socket_memoria;
int socket_kernel_dispatch;
int socket_kernel_interrupt;

// Suponiendo estos valores vienen del archivo de configuración:
int tamanio_pagina;
int niveles_paginacion;
int entradas_por_tabla;
t_list* TLB;
t_list* CACHE; // lista global de entradas de cache



void ejecutar_ciclo_instruccion(int socket_memoria, int pid, int pc) {
    while (true) {
        
        // 1. Fetch
        char *instruccion_str = fetch_instruccion(socket_memoria, pid, pc);
        if (!instruccion_str)
        {
            log_error(loggerCpu, "Error al obtener la instruccion para PID %d - Terminando ejecucion", pid);
            break;
        }

        // Estas cositas las meti dentro de las correspondientes funciones...
        //loguear_instruccion(pcb->pid, instruccion_str); 
        //log_info(loggerCpu, "## PID: %d - FETCH - Program Counter: %d", pcb->pid, pcb->pc); 

        // 2. Decode + Execute
        bool incrementar_pc = decode_and_execute(instruccion_str, pid, pc, socket_memoria);

        // 3. Check Interrupt
        if (check_interrupt())
        {
            log_info(loggerCpu, "## Llega interrupción al puerto Interrupt"); // LOG OBLIGATORIO (Interrupcion recibida)
            enviar_contexto_a_kernel(pid, pc, MOTIVO_INTERRUPCION); // TODO: Hacer funcion
            free(instruccion_str);
            break;
        }

        // MOTIVO_INTERRUPCION: definir!! para informarle al Kernel por qué le estás devolviendo el PCB

        // 4. Incrementar el PC si corresponde
        if (incrementar_pc)
        {
            pc++;
        }

        free(instruccion_str);
    }
}
/*void ejecutar_ciclo_instruccion(t_pcb* pcb, int socket_memoria) {
    while (true) {
        
        // 1. Fetch
        char *instruccion_str = fetch_instruccion(socket_memoria, pcb);
        if (!instruccion_str)
        {
            log_error(logger, "Error al obtener la instruccion para PID %d - Terminando ejecucion", pcb->pid);
            break;
        }

        // Estas cositas las meti dentro de las correspondientes funciones...
        //loguear_instruccion(pcb->pid, instruccion_str); 
        //log_info(logger, "## PID: %d - FETCH - Program Counter: %d", pcb->pid, pcb->pc); 

        // 2. Decode + Execute
        bool incrementar_pc = decode_and_execute(instruccion_str, pcb, socket_memoria);

        // 3. Check Interrupt
        if (check_interrupt())
        {
            log_info(logger, "## Llega interrupción al puerto Interrupt"); // LOG OBLIGATORIO (Interrupcion recibida)
            enviar_contexto_a_kernel(pcb, MOTIVO_INTERRUPCION); // TODO: Hacer funcion
            free(instruccion_str);
            break;
        }

        // MOTIVO_INTERRUPCION: definir!! para informarle al Kernel por qué le estás devolviendo el PCB

        // 4. Incrementar el PC si corresponde
        if (incrementar_pc)
        {
            pcb->pc++;
        }

        free(instruccion_str);
    }
}*/

char *fetch_instruccion(int socket_memoria, int pid, int pc) {       // ESTA BIEN :) :)

    solicitar_instruccion(socket_memoria, pid, pc); 
    char* instruccion = recibir_instruccion(socket_memoria);
    log_info(loggerCpu, "## PID: %d - FETCH - Program Counter: %d", pid, pc); // LOG OBLIGATORIO (Fetch Instruccion)
    
    return instruccion;    
}    
/*char *fetch_instruccion(int socket_memoria, t_pcb* pcb) {       // ESTA BIEN :) :)
    int PID = pcb->pid;
    int program_counter = pcb->pc;
    
    solicitar_instruccion(socket_memoria, PID, program_counter); 
    char* instruccion = recibir_instruccion(socket_memoria);
    log_info(logger, "## PID: %d - FETCH - Program Counter: %d", PID, program_counter); // LOG OBLIGATORIO (Fetch Instruccion)
    
    return instruccion;    
}    
    */

// Devuelve un bool que indica a la CPU si debe incrementar el program_counter
// true: instrucciones normales (NOOP, READ, WRITE)
// false: instrucciones que modifican el PC directamente (GOTO)
// false: instrucciones que suspenden/devuelven el proceso (EXIT, IO, INIT_PROC, DUMP_MEMORY)

bool decode_and_execute(char *instruccion_str, int pid, int pc, int socket_memoria) { // ESTA BIEN :)
    //loguear_instruccion(pcb->pid, instruccion_str); // LOG OBLIGATORIO, le falta los paramentros

    t_parsed_instruccion inst = parse_instruccion(instruccion_str); 

    // Log obligatorio
    loguear_instruccion(pid, instruccion_str);

    // ------------ INSTRUCCIONES DE CPU ---------
    
    if (strcmp(inst.opcode, "NOOP") == 0) {
        usleep(1000 * 500); // 500ms  ???          // tiene que ser el tiempo de ejecucion del ciclo de instruccion 
        //usleep(1000 * TIEMPO_CICLO_INSTRUCCION); // tiene que venir del archivo de configuracion
        return true;
    
    } else if (strcmp(inst.opcode, "GOTO") == 0) {
        if(inst.arg1) {
            int nuevo_pc = atoi(inst.arg1); // atoi tranforma "5" en 5

            // Validar que el nuevo pc este dentro del rango permitido
            if(nuevo_pc >= 0 && nuevo_pc < TAMANIO_MEMORIA) {
                pc = nuevo_pc;
            } else {
                log_warning(loggerCpu, "PID: %d - GOTO fuera de rango: %d", pid, nuevo_pc);
            }

            //pcb->pc = nuevo_pc;
        }
        return false;
        
        //int salto = atoi(inst + 5); // no deberiamos tener un ejecutar GOTO(valor) ? // ARREGLAR LO DEL + 5
        //pcb->pc += salto;
        //return false;

    } else if (strcmp(inst.opcode, "READ") == 0) {
        if(inst.arg1 && inst.arg2) {
            int dir_logica = atoi(inst.arg1);
            int tamanio = atoi(inst.arg2);
            ejecutar_read(pid, dir_logica, tamanio, socket_memoria); // TODO: CREAR FUNCION
        }
        
        return true;

    } else if (strcmp(inst.opcode, "WRITE") == 0) {
        if(inst.arg1 && inst.arg2) {
            int dir_logica = atoi(inst.arg1);
            char* valor = inst.arg2;
            ejecutar_write(pid, dir_logica, valor, socket_memoria); // TODO: CREAR FUNCION
        }
        
        return true;

    // -------- SYSCALLS -----------

    } else if (strcmp(inst.opcode, "EXIT") == 0) {
        enviar_contexto_a_kernel(pid, pc, MOTIVO_EXIT);     // TODO: CREAR FUNCION , ESO PASA CUANDO KERNEL ENVIA UNA INTERRUPCION AL PID QUE SE ESTA EJECUTANDO 
        return false;                                       // SI ESO PASA, SE DEVUELVE EL PID Y EL PC ACTUALIZADO AL KERNEL CON MOTIVO DE INTERRUPCION, SINO SE DESCARTA LA MISMA
    
    } else if (strcmp(inst.opcode, "IO") == 0) {
        if(inst.arg1 && inst.arg2) {
            enviar_syscall_a_kernel(pid, pc, instruccion_str, MOTIVO_SYSCALL); // TODO: CREAR FUNCION
        }
        return false;
    
    } else if (strcmp(inst.opcode, "INIT_PROC") == 0) {
        if(inst.arg1 && inst.arg2) {
            enviar_syscall_a_kernel(pid, pc, instruccion_str, MOTIVO_SYSCALL); // TODO: CREAR FUNCION
        }
        return false;

    } else if (strcmp(inst.opcode, "DUMP_MEMORY") == 0) {
        enviar_syscall_a_kernel(pid, pc, instruccion_str, MOTIVO_SYSCALL); // TODO: CREAR FUNCION     
        return false;

    // -------- INSTRUCCION NO RECONOCIDA -----------

    } else {
        log_warning(loggerCpu, "PID: %d - Instrucción desconocida: %s", pid, instruccion_str); // LOG NO OBLIGATORIO
        return true;
    }
}

/*bool decode_and_execute(char *instruccion_str, t_pcb *pcb, socket_memoria) { // ESTA BIEN :)
    //loguear_instruccion(pcb->pid, instruccion_str); // LOG OBLIGATORIO, le falta los paramentros

    t_parsed_instruccion inst = parse_instruccion(instruccion_str); 

    // Log obligatorio
    loguear_instruccion(pcb->pid, instruccion_str);

    // ------------ INSTRUCCIONES DE CPU ---------
    
    if (strcmp(inst.opcode, "NOOP") == 0) {
        usleep(1000 * 500); // 500ms  ???          // tiene que ser el tiempo de ejecucion del ciclo de instruccion 
        //usleep(1000 * TIEMPO_CICLO_INSTRUCCION); // tiene que venir del archivo de configuracion
        return true;
    
    } else if (strcmp(inst.opcode, "GOTO") == 0) {
        if(inst.arg1) {
            int nuevo_pc = atoi(inst.arg1); // atoi tranforma "5" en 5

            // Validar que el nuevo pc este dentro del rango permitido
            if(nuevo_pc >= 0 && nuevo_pc < TAMANIO_MEMORIA) {
                pcb->pc = nuevo_pc;
            } else {
                log_warning(logger, "PID: %d - GOTO fuera de rango: %d", pcb->pid, nuevo_pc);
            }

            //pcb->pc = nuevo_pc;
        }
        return false;
        
        //int salto = atoi(inst + 5); // no deberiamos tener un ejecurar GOTO(valor) ? // ARREGLAR LO DEL + 5
        //pcb->pc += salto;
        //return false;

    } else if (strcmp(inst.opcode, "READ") == 0) {
        if(inst.arg1 && inst.arg2) {
            int dir_logica = atoi(inst.arg1);
            int tamanio = atoi(inst.arg2);
            ejecutar_read(pcb->pid, dir_logica, tamanio, socket_memoria); // TODO: CREAR FUNCION
        }
        
        return true;

    } else if (strcmp(inst.opcode, "WRITE") == 0) {
        if(inst.arg1 && inst.arg2) {
            int dir_logica = atoi(inst.arg1);
            char* valor = inst.arg2;
            ejecutar_write(pcb->pid, dir_logica, valor, socket_memoria); // TODO: CREAR FUNCION
        }
        
        return true;

    // -------- SYSCALLS -----------

    } else if (strcmp(inst.opcode, "EXIT") == 0) {
        enviar_contexto_a_kernel(pcb, MOTIVO_EXIT);     // TODO: CREAR FUNCION , ESO PASA CUANDO KERNEL ENVIA UNA INTERRUPCION AL PID QUE SE ESTA EJECUTANDO 
        return false;                                   // SI ESO PASA, SE DEVUELVE EL PID Y EL PC ACTUALIZADO AL KERNEL CON MOTIVO DE INTERRUPCION, SINO SE DESCARTA LA MISMA
    
    } else if (strcmp(inst.opcode, "IO") == 0) {
        if(inst.arg1 && inst.arg2) {
            enviar_syscall_a_kernel(pcb, instruccion_str, MOTIVO_SYSCALL); // TODO: CREAR FUNCION
        }
        return false;
    
    } else if (strcmp(inst.opcode, "INIT_PROC") == 0) {
        if(inst.arg1 && inst.arg2) {
            enviar_syscall_a_kernel(pcb, instruccion_str, MOTIVO_SYSCALL); // TODO: CREAR FUNCION
        }
        return false;

    } else if (strcmp(inst.opcode, "DUMP_MEMORY") == 0) {
        enviar_syscall_a_kernel(pcb, instruccion_str, MOTIVO_SYSCALL); // TODO: CREAR FUNCION     
        return false;

    // -------- INSTRUCCION NO RECONOCIDA -----------

    } else {
        log_warning(logger, "PID: %d - Instrucción desconocida: %s", pcb->pid, instruccion_str); // LOG NO OBLIGATORIO
        return true;
    }
}*/

/* LO MOVÍ A INSTRUCCIONES.H PQ LO NECESITO EN KERNEL
// Parser simple que separa la instruccion en part
// Funcion auxiliar de parseo (parsing function, convierte un string en una estructura usable)
// EJEMPLO: "WRITE 10 HOLA" --> "WRITE" "10" "HOLA"

t_parsed_instruccion parse_instruccion(const char* instruccion_str) {
    t_parsed_instruccion resultado;

    char* copia = strdup(instruccion_str); // copiamos porque strtok modifica
    
    resultado.opcode = strtok(copia, " ");
    resultado.arg1 = strtok(NULL, " ");
    resultado.arg2 = strtok(NULL, "");

    // Asegurar que los parametros NO sean NULL
    if(!resultado.arg1) resultado.arg1 = strdup("-");
    if(!resultado.arg2) resultado.arg2 = strdup("-");

    return resultado;
}   */

// Funcion auxiliar para separa el nombre de los parametros
// Ejemplitos
// loguear_instruccion(3, "WRITE 20 hola");
// -> ## PID: 3 - Ejecutando: WRITE - 20 hola

// loguear_instruccion(1, "NOOP");
//  -> ## PID: 1 - Ejecutando: NOOP - -
void loguear_instruccion(int pid, const char *instruccion_str)
{
    char instruccion_copia[256];
    strncpy(instruccion_copia, instruccion_str, sizeof(instruccion_copia));
    instruccion_copia[sizeof(instruccion_copia) - 1] = '\0';

    char *nombre = strtok(instruccion_copia, " ");
    char *parametros = strtok(NULL, "");

    log_info(loggerCpu, "## PID: %d - Ejecutando: %s - %s", pid, nombre, parametros ? parametros : "Sin parametros"); // LOG OBLIGATORIO (Instruccion Ejecutada)
}

// Debe verificar si llego una interripcion del Kernel por el socket de interruciones
// Este socket NO puede bloquear la ejecucion de la CPU, usamos select() con timeout cero
bool check_interrupt()
{
    fd_set read_fds;
    struct timeval timeout = {0}; // Timeout 0 -> no bloquea

    FD_ZERO(&read_fds);
    FD_SET(socket_kernel_interrupt, &read_fds);

    // select() revisa si hay algo sin bloquear
    int result = select(socket_kernel_interrupt + 1, &read_fds, NULL, NULL, &timeout);
    // Si llega algo -> se limpia con recv() y se devuelve true
    // SI no hay nada -> se devuelve false

    if(result > 0 && FD_ISSET(socket_kernel_interrupt, &read_fds)) {
        // Leer y limpiar el mensaje si hay algo
        int motivo;
        recv(socket_kernel_interrupt, &motivo, sizeof(int));
        log_info(loggerCpu, "## Llega interrupción al puerto Interrupt");
        return true;
    }
    return false;

}

// ---- FUNCIONES AUXLIARES ---

void enviar_contexto_a_kernel(int pid, int pc, int motivo) { // ESTA BIEN PERO REVISAR LO DEL BUFFER

    enviar_pid_pc_motivo(socket_kernel_dispatch, pid, pc, motivo);

    // LOG NO OBLIGATORIO PERO ME PERMITE VISUALIZAR QUE ESTA PASANDO
    log_info(loggerCpu, "## PID: %d - Devolviendo contexto al Kernel con PC = %d y motivo = %d", 
    pid, pc, motivo);
}

/*void enviar_contexto_a_kernel(t_pcb *pcb, int motivo) { // ESTA BIEN PERO REVISAR LO DEL BUFFER
    
    t_buffer *buffer = crear_buffer();
    buffer_pack_int(buffer, pcb->pid);
    buffer_pack_int(buffer, pcb->pc);
    buffer_pack_int(buffer, motivo);
    

    enviar_pid_pc_motivo(socket_kernel_dispatch, pcb->pid, pcb->pc, motivo);
    
    
    t_paquete* paquete = crear_paquete();
    agregar_a_paquete(paquete, pcb->pid, sizeof(int));
    agregar_a_paquete(paquete, pcb->pc, sizeof(int));
    agregar_a_paquete(paquete, motivo, sizeof(string));

    //enviar_paquete(DEVOLVER_PROCESO, buffer, socket_kernel_dispatch);
    enviar_paquete(paquete, socket_kernel_dispatch);
    
    //eliminar_buffer(buffer);
    eliminar_paquete(paquete);
    

    // LOG NO OBLIGATORIO PERO ME PERMITE VISUALIZAR QUE ESTA PASANDO
    log_info(loggerCpu, "## PID: %d - Devolviendo contexto al Kernel con PC = %d y motivo = %d", 
    pcb->pid, pcb->pc, motivo);

}*/

/////////// CHEQUEAR TODO ESTO Y TERMINAR DE HACERLO ///////////

// En READ --> se debe imprimir el valor por consola y en el log.
void ejecutar_read(int pid, int direccion_logica, int tamanio, int socket_memoria){
    int direccion_fisica = traducir_direccion_logica_a_fisica(pid, direccion_logica);

    t_paquete* paquete = crear_paquete();
    agregar_a_paquete(paquete, pid, sizeof(int));
    agregar_a_paquete(paquete, direccion_fisica, sizeof(int));
    agregar_a_paquete(paquete, tamanio, sizeof(int));

    //enviar_paquete(READ_MEMORIA, buffer, socket_memoria);
    enviar_paquete(paquete, socket_memoria); 
    eliminar_paquete(paquete);

    // Esperar respuesta de Memoria: string con los datos leídos
    int size = 0;
    recv(socket_memoria, &size, sizeof(int), MSG_WAITALL); // el recv?

    char* contenido = malloc(size);
    recv(socket_memoria, contenido, size, MSG_WAITALL); // el recv?

    // Mostrar en consola con printf?
    //printf("PID: %d - READ - Dirección física: %d - Contenido: %s\n", pid, direccion_fisica, contenido);

    // LOG OBLIGATORIO (Lectura Memoria)
    log_info(loggerCpu, "PID: %d - Acción: LEER - Dirección Física: %d - Valor: %s", pid, direccion_fisica, contenido);

    free(contenido);
}

// OJO el campo valor/datos será siempre una cadena de caracteres sin espacios.
// En WRITE --> se escribe una cadena sin espacios en la dirección traducida.
void ejecutar_write(int pid, int direccion_logica, char* valor, int socket_memoria){
    
    // 1. Traducir dirección lógica a física usando la MMU simulada
    int direccion_fisica = traducir_direccion_logica_a_fisica(pid, direccion_logica);

    t_paquete* paquete = crear_paquete();
    agregar_a_paquete(paquete, pid, sizeof(int));
    agregar_a_paquete(paquete, direccion_fisica, sizeof(int));
    agregar_a_paquete(paquete, valor, strlen(valor) + 1);

    //enviar_paquete(WRITE_MEMORIA, buffer, socket_memoria); 
    enviar_paquete(paquete, socket_memoria); // COMO SE LO MANDO A SOCKET MEMORIA SI NO TENGO LA VARIABLE    
    
    eliminar_paquete(paquete);
    //eliminar_buffer(buffer);

    // Esperar confirmación (un simple int = 1) 
    int confirmacion = 0;
    recv(socket_memoria, &confirmacion, sizeof(int), MSG_WAITALL); // el recv?

    // LOG OBLIGATORIO (Escritura Memoria)
    log_info(loggerCpu, "PID: %d - Acción: ESCRIBIR - Dirección Física: %d - Valor: %s", pid, direccion_fisica, valor);

}



void enviar_syscall_a_kernel(int pid, int pc, char *instruccion_str, int motivo)
{
    t_paquete* paquete = crear_paquete(DEVOLVER_PROCESO);
    agregar_a_paquete(paquete, &pid, sizeof(int));
    agregar_a_paquete(paquete, &pc, sizeof(int));
    agregar_a_paquete(paquete, instruccion_str, strlen(instruccion_str) + 1);
    agregar_a_paquete(paquete, &motivo, sizeof(int));

    enviar_paquete(paquete, socket_kernel_dispatch);
    eliminar_paquete(paquete);
}

//-------------------------------- MMU / Cache / TLB ----------------------------------
// convierte una direccion logica de un proceso(PID) en una direccion fisica de memoria
int traducir_direccion_logica_a_fisica(int pid, int direccion_logica) {
    int nro_pagina = direccion_logica / tamanio_pagina;     // numero de pagina del proceso logico que estamos accediendo
    int desplazamiento = direccion_logica % tamanio_pagina; // desplazamiento (offset) dentro de esa pagina

    int marco = -1; // para indicar que aún no se ha encontrado el marco correspondiente a la pagina solicitada

    // -- Primero: verificar Cache / TLB ---
    // 1. Cache Hit --> se verifica si esa página ya fue leída antes y está en la caché simulada.
    if (cache_habilitada() && cache_contiene_pagina(pid, nro_pagina)) { // TO DO implementar cache_habilitada y cache_contiene_pagina
        log_info(cpu_logger, "PID: %d - Cache Hit - Pagina: %d", pid, nro_pagina);
        marco = obtener_marco_de_cache(pid, nro_pagina);
    }
    // 2. TLB Hit
    else if (tlb_habilitada() && tlb_contiene_pagina(pid, nro_pagina)) {
        log_info(cpu_logger, "PID: %d - TLB HIT - Pagina: %d", pid, nro_pagina);
        marco = obtener_marco_de_tlb(pid, nro_pagina);
    } 
    // 3. TLB Miss → pedir a Memoria
    else {
        log_info(cpu_logger, "PID: %d - TLB MISS - Pagina: %d", pid, nro_pagina);
    
        int tabla_actual = 0
        // Se recorre la jerarquía de tablas de páginas nivel por nivel.
        for(int nivel = 1; nivel <= niveles_paginacion; nivel++) {
            int entrada = entrada_nivel(nro_pagina, nivel, niveles_paginacion, entradas_por_tabla);
            tabla_actual = solicitar_tabla_o_marco(config_cpu->SOCKET_MEMORIA, pid, tabla_actual, entrada);
        }

        marco = tabla_actual; // contien el marco real en memoria
    }

    // Si se tuvo que ir a memoria (TLB miss), 
    // se almacena la traducción obtenida en TLB y Cache para acelerar futuros accesos a esa página

    // actualizar TLB si está habilitada
    if (tlb_habilitada()) {
        tlb_agregar_entrada(pid, nro_pagina, marco);
    }

    // actualizar Cache si está habilitada
    if (cache_habilitada()) {
        cache_agregar_pagina(pid, nro_pagina, marco);
    }
    
    log_info(cpu_logger, "PID: %d - OBTENER MARCO - Página: %d - Marco: %d", pid, nro_pagina, marco);

    return marco * tamanio_pagina + desplazamiento; // devuelve la direccion fisica
}

int entrada_nivel(int nro_pagina, int nivel, int niveles_totales, int entradas_por_tabla) {
    int divisor = pow(entradas_por_tabla, niveles_totales - nivel);
    int resultado = (nro_pagina/divisor) % entradas_por_tabla;
    return resultado;
}

int solicitar_tabla_o_marco(int socket_memoria, int pid, int tabla_actual, int entrada) {
    //op_code codigo = SOLICITAR_MARCO;
    op_code codigo = RESOLVER_ENTRADA_TABLA; 

    // Enviar código de operación
    send(socket_memoria, &codigo, sizeof(op_code), 0);

    // Enviar pid, tabla actual y entrada (datos necesarios para resolver la direccion)
    send(socket_memoria, &pid, sizeof(int), 0);
    send(socket_memoria, &tabla_actual, sizeof(int), 0);
    send(socket_memoria, &entrada, sizeof(int), 0);

    // Esperar respuesta
    int resultado = -1
    int bytes = recv(socket_memoria, &resultado, sizeof(int), MSG_WAITALL);
    
    if (bytes <= 0) {
        log_error(cpu_logger, "Error al recibir respuesta desde Memoria al consultar tabla/marco");
        return -1; // o puedo 
    }

    return resultado;
}

// Cache y TLB
void inicializar_tlb() {
    TLB = list_create();
}

bool tlb_habilitada() {
    return config_cpu->ENTRADAS_TLB > 0;
}

bool tlb_contiene_pagina(int pid, int pagina) {
    for (int i = 0; i < list_size(TLB); i++) {
        entrada_tlb* e = list_get(TLB, i);
        if (e->pid == pid && e->pagina == pagina) {
            e->ultimo_acceso = time(NULL); // actualizar si usamos LRU
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
    return -1; // debería no pasar si validamos antes
}

void tlb_agregar_entrada(int pid, int pagina, int marco) {
    entrada_tlb* nueva = malloc(sizeof(entrada_tlb));
    nueva->pid = pid;
    nueva->pagina = pagina;
    nueva->marco = marco;
    nueva->ultimo_acceso = time(NULL);

    if (list_size(TLB) >= config_cpu->ENTRADAS_TLB) {
        // Reemplazo: FIFO o LRU
        int index_victima = 0;

        if (strcmp(config_cpu->REEMPLAZO_TLB, "LRU") == 0) {
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

bool cache_habilitada() {
    // Si es mayor a 0, la cache está habilitada
    return config_cpu->ENTRADAS_CACHE > 0;
}

void inicializar_cache() {
    CACHE = list_create();
}

// busca en la cache si existe la pagina para el proceso dado. Si si retorna true , sino , false.
bool cache_contiene_pagina(int pid, int pagina) {
    for (int i = 0; i < list_size(CACHE); i++) {
        entrada_cache* entr_cache = list_get(CACHE, i);
        if (entr_cache->pid == pid && entr_cache->pagina == pagina) {
            // Puedo actualizar campos de uso si uso CLOCK-M o LRU
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
    return -1; // debería no pasar si validamos antes
}
void cache_agregar_pagina(int pid, int pagina, int marco) {
    entrada_cache* nueva = malloc(sizeof(entrada_cache));
    nueva->pid = pid;
    nueva->pagina = pagina;
    nueva->marco = marco;
    nueva->ultimo_acceso = time(NULL);

    if (list_size(CACHE) >= config_cpu->ENTRADAS_CACHE) {
        // Reemplazo: FIFO o LRU
        int index_victima = 0;

        if (strcmp(config_cpu->REEMPLAZO_CACHE, "LRU") == 0) {
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