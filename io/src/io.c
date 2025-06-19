#include "ioutils.h"

int main(int argc, char* argv[]) {
    // Validar argumentos de línea de comandos
    if (argc < 2) {
        printf("Uso: ./bin/io [nombre]\n");
        printf("Ejemplo: ./bin/io DISCO\n");
        return EXIT_FAILURE;
    }
    
    char* nombre_io = argv[1];
    
    saludar("io");
    crear_logger();
    inicializar_config();
    cargar_config();
    
    // Conectar al Kernel
    int conexion_kernel = iniciar_conexion_kernel();
    if (conexion_kernel < 0) {
        log_error(loggerIO, "Error al conectar con el Kernel");
        limpiar_recursos();
        return EXIT_FAILURE;
    }
    
    // Enviar handshake con el nombre del dispositivo
    t_paquete* paquete = crear_paquete(HANDSHAKE_IO); 
    agregar_a_paquete(paquete, nombre_io, strlen(nombre_io) + 1);
    enviar_paquete(paquete, conexion_kernel);
    eliminar_paquete(paquete);
    
    log_info(loggerIO, "Dispositivo IO '%s' conectado al Kernel", nombre_io);

    // Manejar las solicitudes de IO
    manejar_peticiones_io(conexion_kernel);

    // Limpiar recursos
    close(conexion_kernel);
    limpiar_recursos();
    return 0;
}


/*
    char* mensaje = "Hola Kernel, soy IO.";
    t_paquete* paquete = crear_paquete(MENSAJE);
    agregar_a_paquete(paquete, mensaje, strlen(mensaje) + 1);
    enviar_paquete(paquete, conexion_kernel);
    eliminar_paquete(paquete);

    
    */