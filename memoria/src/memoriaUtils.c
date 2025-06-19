#include "memoriaUtils.h"
#include <sys/stat.h>
#include <dirent.h>

// Variables globales
t_config* config = NULL;
t_config_memoria* config_struct;
t_log* loggerMemoria = NULL;

// Variables globales de memoria
void* memoria_principal = NULL;
t_dictionary* procesos_memoria = NULL;
t_list* entradas_swap = NULL;
FILE* archivo_swap = NULL;
int tamanio_memoria = 0;
int tamanio_pagina = 0;
int entradas_por_tabla = 0;
int cantidad_niveles = 0;
int retardo_memoria = 0;
int retardo_swap = 0;

// Inicialización de configuración
void inicializar_config(void) {
    config_struct = malloc(sizeof(t_config_memoria));
    config_struct->modulo = NULL;
    config_struct->puerto_escucha = NULL;
    config_struct->tam_memoria = NULL;
    config_struct->tam_pagina = NULL;
    config_struct->entradas_por_tabla = NULL;
    config_struct->cantidad_niveles = NULL;
    config_struct->retardo_memoria = NULL;
    config_struct->path_swapfile = NULL;
    config_struct->retardo_swap = NULL;
    config_struct->log_level = NULL;
    config_struct->dump_path = NULL;
    config_struct->path_instrucciones = NULL;
}

void cargar_config() {
    config = config_create("memoria.config");
    config_struct->modulo = config_get_string_value(config, "MODULO");
    config_struct->puerto_escucha = config_get_string_value(config, "PUERTO_ESCUCHA");
    config_struct->tam_memoria = config_get_string_value(config, "TAM_MEMORIA");
    config_struct->tam_pagina = config_get_string_value(config, "TAM_PAGINA");
    config_struct->entradas_por_tabla = config_get_string_value(config, "ENTRADAS_POR_TABLA");
    config_struct->cantidad_niveles = config_get_string_value(config, "CANTIDAD_NIVELES");
    config_struct->retardo_memoria = config_get_string_value(config, "RETARDO_MEMORIA");
    config_struct->path_swapfile = config_get_string_value(config, "PATH_SWAPFILE");
    config_struct->retardo_swap = config_get_string_value(config, "RETARDO_SWAP");
    config_struct->log_level = config_get_string_value(config, "LOG_LEVEL");
    config_struct->dump_path = config_get_string_value(config, "DUMP_PATH");
    config_struct->path_instrucciones = config_get_string_value(config, "PATH_INSTRUCCIONES");
    
    // Convertir valores string a enteros
    tamanio_memoria = atoi(config_struct->tam_memoria);
    tamanio_pagina = atoi(config_struct->tam_pagina);
    entradas_por_tabla = atoi(config_struct->entradas_por_tabla);
    cantidad_niveles = atoi(config_struct->cantidad_niveles);
    retardo_memoria = atoi(config_struct->retardo_memoria);
    retardo_swap = atoi(config_struct->retardo_swap);
}

void crear_logger() {
    loggerMemoria = iniciar_logger("memoria.log", "MEMORIA", true, LOG_LEVEL_INFO);
}

t_log* iniciar_logger(char* nombreArhcivoLog, char* nombreLog, bool seMuestraEnConsola, t_log_level nivelDetalle) {
    t_log* nuevo_logger;
    nuevo_logger = log_create(nombreArhcivoLog, nombreLog, seMuestraEnConsola, nivelDetalle);
    if (nuevo_logger == NULL) {
        perror("Error en el logger");
        exit(EXIT_FAILURE);
    }
    return nuevo_logger;
}

// Inicialización de memoria
void inicializar_memoria() {
    memoria_principal = malloc(tamanio_memoria);
    procesos_memoria = dictionary_create();
    entradas_swap = list_create();
    inicializar_swap();
    
    // Crear directorio de dump si no existe
    struct stat st = {0};
    if (stat(config_struct->dump_path, &st) == -1) {
        mkdir(config_struct->dump_path, 0700);
    }
}

// Funciones de SWAP
void inicializar_swap() {
    archivo_swap = fopen(config_struct->path_swapfile, "wb+");
    if (archivo_swap == NULL) {
        log_error(loggerMemoria, "Error al abrir archivo SWAP: %s", config_struct->path_swapfile);
        exit(EXIT_FAILURE);
    }
}

void cerrar_swap() {
    if (archivo_swap != NULL) {
        fclose(archivo_swap);
    }
}

void escribir_pagina_swap(int pid, int pagina, void* datos, int tamanio) {
    usleep(retardo_swap * 1000); // Retardo de SWAP
    
    int posicion = obtener_posicion_swap(pid, pagina);
    if (posicion == -1) {
        // Nueva entrada en SWAP
        fseek(archivo_swap, 0, SEEK_END);
        posicion = ftell(archivo_swap);
        
        t_entrada_swap* nueva_entrada = malloc(sizeof(t_entrada_swap));
        nueva_entrada->pid = pid;
        nueva_entrada->pagina = pagina;
        nueva_entrada->posicion_inicio = posicion;
        nueva_entrada->tamanio = tamanio;
        list_add(entradas_swap, nueva_entrada);
    }
    
    fseek(archivo_swap, posicion, SEEK_SET);
    fwrite(datos, 1, tamanio, archivo_swap);
    fflush(archivo_swap);
}

void leer_pagina_swap(int pid, int pagina, void* buffer, int tamanio) {
    usleep(retardo_swap * 1000); // Retardo de SWAP
    
    int posicion = obtener_posicion_swap(pid, pagina);
    if (posicion != -1) {
        fseek(archivo_swap, posicion, SEEK_SET);
        fread(buffer, 1, tamanio, archivo_swap);
    }
}

int obtener_posicion_swap(int pid, int pagina) {
    for (int i = 0; i < list_size(entradas_swap); i++) {
        t_entrada_swap* entrada = list_get(entradas_swap, i);
        if (entrada->pid == pid && entrada->pagina == pagina) {
            return entrada->posicion_inicio;
        }
    }
    return -1;
}

void liberar_entrada_swap(int pid, int pagina) {
    for (int i = 0; i < list_size(entradas_swap); i++) {
        t_entrada_swap* entrada = list_get(entradas_swap, i);
        if (entrada->pid == pid && entrada->pagina == pagina) {
            list_remove(entradas_swap, i);
            free(entrada);
            break;
        }
    }
}

// Funciones de métricas
t_metricas_proceso* crear_metricas_proceso(int pid) {
    t_metricas_proceso* metricas = malloc(sizeof(t_metricas_proceso));
    metricas->pid = pid;
    metricas->accesos_tablas_paginas = 0;
    metricas->instrucciones_solicitadas = 0;
    metricas->bajadas_swap = 0;
    metricas->subidas_memoria_principal = 0;
    metricas->lecturas_memoria = 0;
    metricas->escrituras_memoria = 0;
    return metricas;
}

void incrementar_metrica_accesos_tablas(int pid) {
    t_proceso_memoria* proceso = dictionary_get(procesos_memoria, &pid);
    if (proceso != NULL) {
        proceso->metricas->accesos_tablas_paginas++;
    }
}

void incrementar_metrica_instrucciones(int pid) {
    t_proceso_memoria* proceso = dictionary_get(procesos_memoria, &pid);
    if (proceso != NULL) {
        proceso->metricas->instrucciones_solicitadas++;
    }
}

void incrementar_metrica_bajadas_swap(int pid) {
    t_proceso_memoria* proceso = dictionary_get(procesos_memoria, &pid);
    if (proceso != NULL) {
        proceso->metricas->bajadas_swap++;
    }
}

void incrementar_metrica_subidas_memoria(int pid) {
    t_proceso_memoria* proceso = dictionary_get(procesos_memoria, &pid);
    if (proceso != NULL) {
        proceso->metricas->subidas_memoria_principal++;
    }
}

void incrementar_metrica_lecturas(int pid) {
    t_proceso_memoria* proceso = dictionary_get(procesos_memoria, &pid);
    if (proceso != NULL) {
        proceso->metricas->lecturas_memoria++;
    }
}

void incrementar_metrica_escrituras(int pid) {
    t_proceso_memoria* proceso = dictionary_get(procesos_memoria, &pid);
    if (proceso != NULL) {
        proceso->metricas->escrituras_memoria++;
    }
}

void log_metricas_proceso(int pid) {
    t_proceso_memoria* proceso = dictionary_get(procesos_memoria, &pid);
    if (proceso != NULL) {
        t_metricas_proceso* m = proceso->metricas;
        log_info(loggerMemoria, "## PID: %d - Proceso Destruido - Métricas - Acc.T.Pag: %d; Inst.Sol.: %d; SWAP: %d; Mem.Prin.: %d; Lec.Mem.: %d; Esc.Mem.: %d",
                 pid, m->accesos_tablas_paginas, m->instrucciones_solicitadas, m->bajadas_swap,
                 m->subidas_memoria_principal, m->lecturas_memoria, m->escrituras_memoria);
    }
}

// Funciones de tablas de páginas
t_entrada_tabla*** crear_tablas_paginas(int niveles, int entradas_por_tabla) {
    t_entrada_tabla*** tablas = malloc(niveles * sizeof(t_entrada_tabla**));
    
    for (int i = 0; i < niveles; i++) {
        tablas[i] = malloc(entradas_por_tabla * sizeof(t_entrada_tabla*));
        for (int j = 0; j < entradas_por_tabla; j++) {
            tablas[i][j] = malloc(sizeof(t_entrada_tabla));
            tablas[i][j]->marco = -1;
            tablas[i][j]->presente = false;
            tablas[i][j]->modificada = false;
            tablas[i][j]->posicion_swap = -1;
        }
    }
    
    return tablas;
}

void destruir_tablas_paginas(t_entrada_tabla*** tablas, int niveles, int entradas_por_tabla) {
    for (int i = 0; i < niveles; i++) {
        for (int j = 0; j < entradas_por_tabla; j++) {
            free(tablas[i][j]);
        }
        free(tablas[i]);
    }
    free(tablas);
}

int resolver_entrada_tabla(int pid, int tabla_actual, int entrada) {
    incrementar_metrica_accesos_tablas(pid);
    usleep(retardo_memoria * 1000); // Retardo de memoria
    
    t_proceso_memoria* proceso = dictionary_get(procesos_memoria, &pid);
    if (proceso == NULL) {
        return -1;
    }
    
    // Simulación de resolución de entrada de tabla
    return tabla_actual * entradas_por_tabla + entrada;
}

// Funciones de acceso a memoria
void leer_memoria(int pid, int direccion_fisica, int tamanio, void* buffer) {
    incrementar_metrica_lecturas(pid);
    usleep(retardo_memoria * 1000); // Retardo de memoria
    
    t_proceso_memoria* proceso = dictionary_get(procesos_memoria, &pid);
    if (proceso != NULL) {
        memcpy(buffer, (char*)proceso->espacio_memoria + direccion_fisica, tamanio);
        log_info(loggerMemoria, "## PID: %d - Lectura - Dir. Física: %d - Tamaño: %d", pid, direccion_fisica, tamanio);
    }
}

void escribir_memoria(int pid, int direccion_fisica, int tamanio, void* datos) {
    incrementar_metrica_escrituras(pid);
    usleep(retardo_memoria * 1000); // Retardo de memoria
    
    t_proceso_memoria* proceso = dictionary_get(procesos_memoria, &pid);
    if (proceso != NULL) {
        memcpy((char*)proceso->espacio_memoria + direccion_fisica, datos, tamanio);
        log_info(loggerMemoria, "## PID: %d - Escritura - Dir. Física: %d - Tamaño: %d", pid, direccion_fisica, tamanio);
    }
}

void leer_pagina_completa(int pid, int direccion_fisica, void* buffer) {
    incrementar_metrica_lecturas(pid);
    usleep(retardo_memoria * 1000);
    
    t_proceso_memoria* proceso = dictionary_get(procesos_memoria, &pid);
    if (proceso != NULL) {
        memcpy(buffer, (char*)proceso->espacio_memoria + direccion_fisica, tamanio_pagina);
    }
}

void actualizar_pagina_completa(int pid, int direccion_fisica, void* datos) {
    incrementar_metrica_escrituras(pid);
    usleep(retardo_memoria * 1000);
    
    t_proceso_memoria* proceso = dictionary_get(procesos_memoria, &pid);
    if (proceso != NULL) {
        memcpy((char*)proceso->espacio_memoria + direccion_fisica, datos, tamanio_pagina);
    }
}

// Funciones de instrucciones
void cargar_instrucciones_proceso(t_proceso_memoria* proceso) {
    char ruta_completa[512];
    sprintf(ruta_completa, "%s/%s", config_struct->path_instrucciones, proceso->archivo_instrucciones);
    
    FILE* archivo = fopen(ruta_completa, "r");
    if (archivo == NULL) {
        log_error(loggerMemoria, "Error al abrir archivo de instrucciones: %s", ruta_completa);
        return;
    }
    
    proceso->instrucciones = list_create();
    char linea[256];
    
    while (fgets(linea, sizeof(linea), archivo)) {
        linea[strcspn(linea, "\n")] = 0;
        char* instruccion = strdup(linea);
        list_add(proceso->instrucciones, instruccion);
    }
    
    proceso->cantidad_instrucciones = list_size(proceso->instrucciones);
    fclose(archivo);
}

char* obtener_instruccion(int pid, int pc) {
    incrementar_metrica_instrucciones(pid);
    
    t_proceso_memoria* proceso = dictionary_get(procesos_memoria, &pid);
    if (proceso == NULL || pc >= proceso->cantidad_instrucciones) {
        return NULL;
    }
    
    char* instruccion = list_get(proceso->instrucciones, pc);
    log_info(loggerMemoria, "## PID: %d - Obtener instrucción: %d - Instrucción: %s", pid, pc, instruccion);
    
    return strdup(instruccion);
}

// Funciones de gestión de procesos
t_proceso_memoria* crear_proceso_memoria(int pid, int tamanio, char* archivo_instrucciones) {
    t_proceso_memoria* proceso = malloc(sizeof(t_proceso_memoria));
    proceso->pid = pid;
    proceso->tamanio = tamanio;
    proceso->espacio_memoria = malloc(tamanio);
    proceso->tablas_paginas = crear_tablas_paginas(cantidad_niveles, entradas_por_tabla);
    proceso->metricas = crear_metricas_proceso(pid);
    proceso->archivo_instrucciones = strdup(archivo_instrucciones);
    proceso->instrucciones = NULL;
    proceso->cantidad_instrucciones = 0;
    
    cargar_instrucciones_proceso(proceso);
    
    dictionary_put(procesos_memoria, &proceso->pid, proceso);
    log_info(loggerMemoria, "## PID: %d - Proceso Creado - Tamaño: %d", pid, tamanio);
    
    return proceso;
}

void destruir_proceso_memoria(int pid) {
    t_proceso_memoria* proceso = dictionary_get(procesos_memoria, &pid);
    if (proceso != NULL) {
        log_metricas_proceso(pid);
        
        free(proceso->espacio_memoria);
        destruir_tablas_paginas(proceso->tablas_paginas, cantidad_niveles, entradas_por_tabla);
        free(proceso->metricas);
        free(proceso->archivo_instrucciones);
        
        if (proceso->instrucciones != NULL) {
            list_clean_and_destroy_elements(proceso->instrucciones, free);
        }
        
        dictionary_remove(procesos_memoria, &pid);
        free(proceso);
    }
}

void suspender_proceso(int pid) {
    t_proceso_memoria* proceso = dictionary_get(procesos_memoria, &pid);
    if (proceso != NULL) {
        int paginas = (proceso->tamanio + tamanio_pagina - 1) / tamanio_pagina;
        for (int i = 0; i < paginas; i++) {
            void* buffer_pagina = malloc(tamanio_pagina);
            int direccion_inicio = i * tamanio_pagina;
            int tamanio_efectivo = (direccion_inicio + tamanio_pagina > proceso->tamanio) ? 
                                   proceso->tamanio - direccion_inicio : tamanio_pagina;
            
            memcpy(buffer_pagina, (char*)proceso->espacio_memoria + direccion_inicio, tamanio_efectivo);
            escribir_pagina_swap(pid, i, buffer_pagina, tamanio_efectivo);
            incrementar_metrica_bajadas_swap(pid);
            
            free(buffer_pagina);
        }
        
        free(proceso->espacio_memoria);
        proceso->espacio_memoria = NULL;
    }
}

void desuspender_proceso(int pid) {
    t_proceso_memoria* proceso = dictionary_get(procesos_memoria, &pid);
    if (proceso != NULL && proceso->espacio_memoria == NULL) {
        proceso->espacio_memoria = malloc(proceso->tamanio);
        
        int paginas = (proceso->tamanio + tamanio_pagina - 1) / tamanio_pagina;
        for (int i = 0; i < paginas; i++) {
            void* buffer_pagina = malloc(tamanio_pagina);
            int direccion_inicio = i * tamanio_pagina;
            int tamanio_efectivo = (direccion_inicio + tamanio_pagina > proceso->tamanio) ? 
                                   proceso->tamanio - direccion_inicio : tamanio_pagina;
            
            leer_pagina_swap(pid, i, buffer_pagina, tamanio_efectivo);
            memcpy((char*)proceso->espacio_memoria + direccion_inicio, buffer_pagina, tamanio_efectivo);
            incrementar_metrica_subidas_memoria(pid);
            
            free(buffer_pagina);
            liberar_entrada_swap(pid, i);
        }
    }
}

void finalizar_proceso(int pid) {
    for (int i = 0; i < list_size(entradas_swap); i++) {
        t_entrada_swap* entrada = list_get(entradas_swap, i);
        if (entrada->pid == pid) {
            list_remove(entradas_swap, i);
            free(entrada);
            i--;
        }
    }
    
    destruir_proceso_memoria(pid);
}

// Funciones de memory dump
void crear_memory_dump(int pid) {
    t_proceso_memoria* proceso = dictionary_get(procesos_memoria, &pid);
    if (proceso == NULL) {
        return;
    }
    
    log_info(loggerMemoria, "## PID: %d - Memory Dump solicitado", pid);
    
    time_t timestamp = time(NULL);
    char nombre_archivo[256];
    sprintf(nombre_archivo, "%s/%d-%ld.dmp", config_struct->dump_path, pid, timestamp);
    
    FILE* archivo_dump = fopen(nombre_archivo, "wb");
    if (archivo_dump != NULL) {
        fwrite(proceso->espacio_memoria, 1, proceso->tamanio, archivo_dump);
        fclose(archivo_dump);
    }
}

// Funciones de manejo de conexiones
void atender_solicitud_marco(int socket_cpu) {
    int pid, tabla_actual, entrada;
    recv(socket_cpu, &pid, sizeof(int), MSG_WAITALL);
    recv(socket_cpu, &tabla_actual, sizeof(int), MSG_WAITALL);
    recv(socket_cpu, &entrada, sizeof(int), MSG_WAITALL);
    
    int marco = resolver_entrada_tabla(pid, tabla_actual, entrada);
    send(socket_cpu, &marco, sizeof(int), 0);
}

void atender_solicitud_instruccion(int socket_cpu) {
    int pid, pc;
    recv(socket_cpu, &pid, sizeof(int), MSG_WAITALL);
    recv(socket_cpu, &pc, sizeof(int), MSG_WAITALL);
    
    char* instruccion = obtener_instruccion(pid, pc);
    if (instruccion != NULL) {
        int tamanio = strlen(instruccion) + 1;
        send(socket_cpu, &tamanio, sizeof(int), 0);
        send(socket_cpu, instruccion, tamanio, 0);
        free(instruccion);
    } else {
        int tamanio = 0;
        send(socket_cpu, &tamanio, sizeof(int), 0);
    }
}

void atender_solicitud_lectura(int socket_cpu) {
    int pid, direccion_fisica, tamanio;
    recv(socket_cpu, &pid, sizeof(int), MSG_WAITALL);
    recv(socket_cpu, &direccion_fisica, sizeof(int), MSG_WAITALL);
    recv(socket_cpu, &tamanio, sizeof(int), MSG_WAITALL);
    
    void* buffer = malloc(tamanio);
    leer_memoria(pid, direccion_fisica, tamanio, buffer);
    
    send(socket_cpu, &tamanio, sizeof(int), 0);
    send(socket_cpu, buffer, tamanio, 0);
    free(buffer);
}

void atender_solicitud_escritura(int socket_cpu) {
    int pid, direccion_fisica, tamanio;
    recv(socket_cpu, &pid, sizeof(int), MSG_WAITALL);
    recv(socket_cpu, &direccion_fisica, sizeof(int), MSG_WAITALL);
    recv(socket_cpu, &tamanio, sizeof(int), MSG_WAITALL);
    
    void* datos = malloc(tamanio);
    recv(socket_cpu, datos, tamanio, MSG_WAITALL);
    
    escribir_memoria(pid, direccion_fisica, tamanio, datos);
    
    int confirmacion = 1;
    send(socket_cpu, &confirmacion, sizeof(int), 0);
    free(datos);
}

void atender_solicitud_lectura_pagina(int socket_cpu) {
    int pid, direccion_fisica;
    recv(socket_cpu, &pid, sizeof(int), MSG_WAITALL);
    recv(socket_cpu, &direccion_fisica, sizeof(int), MSG_WAITALL);
    
    void* buffer = malloc(tamanio_pagina);
    leer_pagina_completa(pid, direccion_fisica, buffer);
    
    send(socket_cpu, buffer, tamanio_pagina, 0);
    free(buffer);
}

void atender_solicitud_escritura_pagina(int socket_cpu) {
    int pid, direccion_fisica;
    recv(socket_cpu, &pid, sizeof(int), MSG_WAITALL);
    recv(socket_cpu, &direccion_fisica, sizeof(int), MSG_WAITALL);
    
    void* datos = malloc(tamanio_pagina);
    recv(socket_cpu, datos, tamanio_pagina, MSG_WAITALL);
    
    actualizar_pagina_completa(pid, direccion_fisica, datos);
    
    int confirmacion = 1;
    send(socket_cpu, &confirmacion, sizeof(int), 0);
    free(datos);
}

void atender_solicitud_memory_dump(int socket_cpu) {
    int pid;
    recv(socket_cpu, &pid, sizeof(int), MSG_WAITALL);
    
    crear_memory_dump(pid);
    
    int confirmacion = 1;
    send(socket_cpu, &confirmacion, sizeof(int), 0);
}

void atender_solicitud_iniciar_proceso(int socket_kernel) {
    int pid, tamanio;
    recv(socket_kernel, &pid, sizeof(int), MSG_WAITALL);
    recv(socket_kernel, &tamanio, sizeof(int), MSG_WAITALL);
    
    int tamanio_nombre;
    recv(socket_kernel, &tamanio_nombre, sizeof(int), MSG_WAITALL);
    char* archivo_instrucciones = malloc(tamanio_nombre);
    recv(socket_kernel, archivo_instrucciones, tamanio_nombre, MSG_WAITALL);
    
    crear_proceso_memoria(pid, tamanio, archivo_instrucciones);
    free(archivo_instrucciones);
    
    int resultado = 1; // OK
    send(socket_kernel, &resultado, sizeof(int), 0);
}

void atender_solicitud_suspender_proceso(int socket_kernel) {
    int pid;
    recv(socket_kernel, &pid, sizeof(int), MSG_WAITALL);
    
    suspender_proceso(pid);
    
    int resultado = 1; // OK
    send(socket_kernel, &resultado, sizeof(int), 0);
}

void atender_solicitud_desuspender_proceso(int socket_kernel) {
    int pid;
    recv(socket_kernel, &pid, sizeof(int), MSG_WAITALL);
    
    desuspender_proceso(pid);
    
    int resultado = 1; // OK
    send(socket_kernel, &resultado, sizeof(int), 0);
}

void atender_solicitud_finalizar_proceso(int socket_kernel) {
    int pid;
    recv(socket_kernel, &pid, sizeof(int), MSG_WAITALL);
    
    finalizar_proceso(pid);
    
    int resultado = 1; // OK
    send(socket_kernel, &resultado, sizeof(int), 0);
}

// Servidores
void* iniciar_servidor_cpu(void* arg) {
    int fd_sv = crear_servidor(config_struct->puerto_escucha);
    log_info(loggerMemoria, "## CPU Conectado - FD del socket: %d", fd_sv);
    
    while (1) {
        int fd_conexion = esperar_cliente(fd_sv, "CPU", loggerMemoria);
        
        int operacion = recibir_operacion(fd_conexion);
        switch(operacion) {
            case SOLICITAR_INSTRUCCION:
                atender_solicitud_instruccion(fd_conexion);
                break;
            case RESOLVER_ENTRADA_TABLA:
                atender_solicitud_marco(fd_conexion);
                break;
            case READ_MEMORIA:
                atender_solicitud_lectura(fd_conexion);
                break;
            case WRITE_MEMORIA:
                atender_solicitud_escritura(fd_conexion);
                break;
            case READ_PAGINA:
                atender_solicitud_lectura_pagina(fd_conexion);
                break;
            case WRITE_PAGINA:
                atender_solicitud_escritura_pagina(fd_conexion);
                break;
            case DUMP_MEMORY:
                atender_solicitud_memory_dump(fd_conexion);
                break;
            default:
                log_error(loggerMemoria, "Operación no reconocida: %d", operacion);
                break;
        }
        
        close(fd_conexion);
    }
    close(fd_sv);
}

void* iniciar_servidor_kernel(void* arg) {
    int fd_sv = crear_servidor(config_struct->puerto_escucha);
    log_info(loggerMemoria, "## Kernel Conectado - FD del socket: %d", fd_sv);
    
    while (1) {
        int fd_conexion = esperar_cliente(fd_sv, "KERNEL", loggerMemoria);
        
        int operacion = recibir_operacion(fd_conexion);
        switch(operacion) {
            case INICIAR_PROCESO:
                atender_solicitud_iniciar_proceso(fd_conexion);
                break;
            case SUSPENDER_PROCESO:
                atender_solicitud_suspender_proceso(fd_conexion);
                break;
            case DESUSPENDER_PROCESO:
                atender_solicitud_desuspender_proceso(fd_conexion);
                break;
            case FINALIZAR_PROCESO:
                atender_solicitud_finalizar_proceso(fd_conexion);
                break;
            default:
                log_error(loggerMemoria, "Operación no reconocida: %d", operacion);
                break;
        }
        
        close(fd_conexion);
    }
    close(fd_sv);
}

void liberar_conexion(int fd) {
    close(fd);
} 