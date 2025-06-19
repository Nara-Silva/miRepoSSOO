#include <utils/protocolo.h>

void destruir_buffer (t_buffer* buffer) {
    free(buffer->stream);
    free(buffer);
}

void enviar_pcb(int fd, t_pcb* pcb) {
    // Calculá el tamaño total a enviar
    int size = sizeof(int) * 3 + sizeof(int) * CANTIDAD_ESTADOS * 2 + sizeof(int); // pid, size, pc, metricas_estado, metricas_tiempo, estado
    void* buffer = malloc(size);
    int offset = 0;

    memcpy(buffer + offset, &pcb->pid, sizeof(int));
    offset += sizeof(int);

    memcpy(buffer + offset, &pcb->size, sizeof(int));
    offset += sizeof(int);

    memcpy(buffer + offset, &pcb->pc, sizeof(int));
    offset += sizeof(int);

    memcpy(buffer + offset, pcb->metricas_estado, sizeof(int) * CANTIDAD_ESTADOS);
    offset += sizeof(int) * CANTIDAD_ESTADOS;

    memcpy(buffer + offset, pcb->metricas_tiempo, sizeof(int) * CANTIDAD_ESTADOS);
    offset += sizeof(int) * CANTIDAD_ESTADOS;

    memcpy(buffer + offset, &pcb->estado, sizeof(int));
    offset += sizeof(int);

    send(fd, buffer, size, 0);
    free(buffer);
}

// Enviar pid, pc y motivo en un solo paquete
void enviar_pid_pc_motivo(int fd, int pid, int pc, int motivo) {
    t_paquete* paquete = crear_paquete(DEVOLVER_PROCESO);
    agregar_a_paquete(paquete, &pid, sizeof(int));
    agregar_a_paquete(paquete, &pc, sizeof(int));
    agregar_a_paquete(paquete, &motivo, sizeof(int));
    enviar_paquete(paquete, fd);
    eliminar_paquete(paquete);
}

//Struct de pc y pid (tal vez motivo) idea de Cande (la unica (idea)) :)                     <-------------- ACA LOCO
/*
typedef struct {
    int pid;
    int pc;
    int motivo; // Motivo de devolución (SYSCALL, EXIT, INTERRUPCIÓN, etc)
    char* instruccion; // string serializado, puede estar vacío si no aplica
} t_contexto_de_ejecucion;
*/
//GRACIAS CANDE, NOS SALVASTE

// Recibir pid, pc y motivo de un paquete
//DEBERIA SER void recibir_pid_pc_motivo(int fd, int* pid, int* pc, char* instruccion_str, int* motivo)
void recibir_pid_pc_motivo(int fd, int* pid, int* pc, int* motivo) {
    int size;
    void* buffer = recibir_buffer(&size, fd);
    int offset = 0;
    memcpy(pid, buffer + offset, sizeof(int));
    offset += sizeof(int);
    memcpy(pc, buffer + offset, sizeof(int));
    offset += sizeof(int);
    memcpy(motivo, buffer + offset, sizeof(int));
    free(buffer);
}

t_pcb* recibir_pcb(int fd) {
    int size = sizeof(int) * 3 + sizeof(int) * CANTIDAD_ESTADOS * 2 + sizeof(int);
    void* buffer = malloc(size);
    int bytes = recv(fd, buffer, size, MSG_WAITALL);
    if (bytes <= 0) {
        free(buffer);
        return NULL;
    }

    t_pcb* pcb = malloc(sizeof(t_pcb));
    int offset = 0;

    memcpy(&pcb->pid, buffer + offset, sizeof(int));
    offset += sizeof(int);

    memcpy(&pcb->size, buffer + offset, sizeof(int));
    offset += sizeof(int);

    memcpy(&pcb->pc, buffer + offset, sizeof(int));
    offset += sizeof(int);

    memcpy(pcb->metricas_estado, buffer + offset, sizeof(int) * CANTIDAD_ESTADOS);
    offset += sizeof(int) * CANTIDAD_ESTADOS;

    memcpy(pcb->metricas_tiempo, buffer + offset, sizeof(int) * CANTIDAD_ESTADOS);
    offset += sizeof(int) * CANTIDAD_ESTADOS;

    memcpy(&pcb->estado, buffer + offset, sizeof(int));
    offset += sizeof(int);

    free(buffer);
    return pcb;
}

t_paquete* crear_paquete(op_code operacion) {
	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->cod_op = operacion;
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = 0;
	paquete->buffer->stream = NULL;
	return paquete;
}

void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio) {
	paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + tamanio);
	memcpy(paquete->buffer->stream + paquete->buffer->size, valor, tamanio);
	paquete->buffer->size += tamanio;
}

void* serializar_paquete(t_paquete* paquete, int bytes) {
	void* buffer_serializado = malloc(bytes);
	int desplazamiento = 0;

	memcpy(buffer_serializado + desplazamiento, &(paquete->cod_op), sizeof(op_code));
	desplazamiento += sizeof(op_code);

	memcpy(buffer_serializado + desplazamiento, &(paquete->buffer->size), sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	memcpy(buffer_serializado + desplazamiento, paquete->buffer->stream, paquete->buffer->size);

	return buffer_serializado;
}

void enviar_paquete(t_paquete* paquete, int socket_cliente) {
	int bytes = sizeof(paquete->cod_op) + sizeof(uint32_t) + paquete->buffer->size;
	void* a_enviar = serializar_paquete(paquete, bytes);
	send(socket_cliente, a_enviar, bytes, 0);
	free(a_enviar);
}

void eliminar_paquete(t_paquete* paquete) {
	destruir_buffer(paquete->buffer);
	free(paquete);
}

// ======== ENVÍO Y RECEPCIÓN DE MENSAJES SIMPLES ========

void enviar_mensaje(char* mensaje, int socket_cliente) {
	t_paquete* paquete = crear_paquete(MENSAJE);
	agregar_a_paquete(paquete, mensaje, strlen(mensaje) + 1);
	enviar_paquete(paquete, socket_cliente);
	eliminar_paquete(paquete);
}

void recibir_mensaje(t_log* logger, int socket_cliente) {
	int size;
	char* buffer = recibir_buffer(&size, socket_cliente);
	log_info(logger, "Me llegó el mensaje: %s", buffer);
	free(buffer);
}

void recibir_proceso(t_log* logger, int socket_cliente) {
	int size;
	char* buffer = recibir_buffer(&size, socket_cliente);
	log_info(logger, "Me llegó el mensaje: %s", buffer);
	free(buffer);
}

// ======== UTILIDADES PARA RECEPCIÓN DE DATOS ========

void* recibir_buffer(int* size, int socket_cliente) {
	void* buffer;

	recv(socket_cliente, size, sizeof(uint32_t), MSG_WAITALL);
	buffer = malloc(*size);
	recv(socket_cliente, buffer, *size, MSG_WAITALL);

	return buffer;
}

int recibir_operacion(int socket_cliente) {
	int cod_op;
	if (recv(socket_cliente, &cod_op, sizeof(int), MSG_WAITALL) > 0)
		return cod_op;
//	close(socket_cliente);
	return -1;
}