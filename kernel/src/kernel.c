#include "kernelUtils.h"

int main(int argc, char* argv[]) {
    // Validar argumentos de línea de comandos
    if (argc < 3) {
        printf("Uso: ./bin/kernel [archivo_pseudocodigo] [tamanio_proceso] [...args]\n");
        return EXIT_FAILURE;
    }
    
    char* archivo_pseudocodigo = argv[1];
    int tamanio_proceso = atoi(argv[2]);
    
    saludar("kernel");
    crear_logger();
    inicializar_config();
    cargar_config();
    inicializar_listasDeEstados();
    inicializar_diccionarios();
    inicializar_mutex_pid();
    inicializar_semaforos();
    
    // Crear proceso inicial
    static int pid_global = 0;
    t_pcb* proceso_inicial = crear_pcb(pid_global, tamanio_proceso);
    log_info(loggerKernel, "## (%d) Se crea el proceso - Estado: NEW", proceso_inicial->pid);
    agregar_a_new(proceso_inicial);
    
    // Crear hilos para diferentes funcionalidades
    pthread_t hilo_dispatch;
    pthread_t hilo_interrupt;
    pthread_t hilo_io;
    pthread_t hilo_planificador_largo_plazo;
    pthread_t hilo_planificador_corto_plazo;
    pthread_t hilo_planificador_mediano_plazo;

    // Iniciar servidores
    pthread_create(&hilo_dispatch, NULL, iniciar_servidor_dispatch, NULL);
    pthread_create(&hilo_interrupt, NULL, iniciar_servidor_interrupt, NULL);
    pthread_create(&hilo_io, NULL, iniciar_servidor_io, NULL);
    
    // Iniciar planificadores
    pthread_create(&hilo_planificador_largo_plazo, NULL, planificador_largo_plazo, NULL);
    pthread_create(&hilo_planificador_corto_plazo, NULL, planificador_corto_plazo, NULL);
    pthread_create(&hilo_planificador_mediano_plazo, NULL, planificador_mediano_plazo, NULL);
    
    // Esperar a que terminen los hilos principales
    pthread_join(hilo_dispatch, NULL);
    pthread_join(hilo_interrupt, NULL);
    pthread_join(hilo_io, NULL);
    pthread_join(hilo_planificador_largo_plazo, NULL);
    pthread_join(hilo_planificador_corto_plazo, NULL);
    pthread_join(hilo_planificador_mediano_plazo, NULL);

    // Limpiar recursos
    liberar_semaforos();
    destruir_diccionarios();
    terminar_programa(-1, loggerKernel, config);
    
    return 0;
}
