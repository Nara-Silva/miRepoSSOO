#include "cpuUtils.h"
#include "tlb.h"
#include "cache.h"
#include "mmu.h"

int main(int argc, char* argv[]) {
    // Verificar que se pase el identificador de CPU
    if (argc != 2) {
        printf("Uso: ./bin/cpu [identificador]\n");
        return 1;
    }
    
    char* identificador_cpu = argv[1];
    
    saludar("cpu");
    crear_logger();
    inicializar_config();
    cargar_config();
    
    // Inicializar TLB y Cache
    inicializar_tlb();
    inicializar_cache();
    
    pthread_t hilo_kernel, hilo_memoria;

    pthread_create(&hilo_memoria, NULL, iniciar_conexion_memoria, NULL);
    pthread_create(&hilo_kernel, NULL, iniciar_conexion_kernel, NULL);

    pthread_join(hilo_kernel, NULL);
    pthread_join(hilo_memoria, NULL);

    pthread_exit(hilo_kernel);
    pthread_exit(hilo_memoria);

    // Crear conexión a Memoria y guardar el socket
    int socket_memoria = crear_conexion(config_struct->ip_memoria, config_struct->puerto_memoria);

    // Crear conexión a Kernel
    int socket_kernel = crear_conexion(config_struct->ip_kernel, config_struct->puerto_kernel_dispatch);

    // Esperar y recibir el PID, PC y motivo
    int pid, pc, motivo;
    recibir_pid_pc_motivo(socket_kernel, &pid, &pc, &motivo);
    
    // Cuando recibas un PCB para ejecutar, pasás el socket_memoria:
    ejecutar_ciclo_instruccion(socket_memoria, pid, pc);
    
    // Limpiar recursos
    limpiar_tlb();
    limpiar_cache();
    
    terminar_programa(-1, loggerCpu, config);
    return 0;
}