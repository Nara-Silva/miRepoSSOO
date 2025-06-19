#include "memoriaUtils.h"


int main(int argc, char* argv[]) {
    saludar("memoria");
    crear_logger();
    inicializar_config();
    cargar_config();
    
    // Inicializar memoria y SWAP
    inicializar_memoria();
    
    pthread_t hilo_cpu, hilo_kernel;
  
    pthread_create(&hilo_kernel,NULL,iniciar_servidor_kernel, NULL);
    pthread_create(&hilo_cpu,NULL,iniciar_servidor_cpu,NULL);

    pthread_join(hilo_cpu,NULL);
    pthread_join(hilo_kernel,NULL);

    // Limpiar recursos
    cerrar_swap();
    if (memoria_principal != NULL) {
        free(memoria_principal);
    }
    if (procesos_memoria != NULL) {
        dictionary_destroy_and_destroy_elements(procesos_memoria, free);
    }
    if (entradas_swap != NULL) {
        list_clean_and_destroy_elements(entradas_swap, free);
    }

    terminar_programa(-1,loggerMemoria,config);
    return 0;
}
