# Sistema Operativo Simulado - TP SSOO

Este proyecto implementa un sistema operativo simulado con los siguientes módulos:
- **CPU**: Ejecuta instrucciones y maneja el ciclo de instrucciones
- **Memoria**: Gestiona memoria principal y SWAP
- **Kernel**: Planifica procesos y coordina todos los módulos
- **IO**: Simula dispositivos de entrada/salida

## Estructura del Proyecto

```
tp-2025-1c-nullpointers/
├── cpu/           # Módulo CPU
├── memoria/       # Módulo Memoria
├── kernel/        # Módulo Kernel
├── io/           # Módulo IO
└── utils/        # Utilidades compartidas
```

## Compilación

### 1. Compilar todos los módulos

```bash
# Desde la raíz del proyecto
cd tp-2025-1c-nullpointers

# Compilar CPU
cd cpu && make clean && make && cd ..

# Compilar Memoria
cd memoria && make clean && make && cd ..

# Compilar Kernel
cd kernel && make clean && make && cd ..

# Compilar IO
cd io && make clean && make && cd ..
```

### 2. Verificar que se crearon los ejecutables

```bash
ls -la cpu/bin/cpu
ls -la memoria/bin/memoria
ls -la kernel/bin/kernel
ls -la io/bin/io
```

## Configuración de Puertos

Los módulos se comunican a través de los siguientes puertos:

| Módulo | Puerto Dispatch | Puerto Interrupt | Puerto IO |
|--------|----------------|------------------|-----------|
| Kernel | 8001 | 8004 | 8003 |
| CPU | - | - | - |
| Memoria | - | - | - |
| IO | - | - | - |

## Orden de Inicio y Conexión

### Paso 1: Iniciar Memoria
```bash
cd tp-2025-1c-nullpointers/memoria
./bin/memoria
```

**Logs esperados:**
```
INFO - Memoria iniciada en puerto 8002
INFO - SWAP inicializado
```

### Paso 2: Iniciar Kernel
```bash
cd tp-2025-1c-nullpointers/kernel
./bin/kernel archivo_pseudocodigo.txt 1024
```

**Logs esperados:**
```
INFO - Kernel iniciado
INFO - Servidor de dispatch iniciado en puerto 8001
INFO - Servidor de interrupt iniciado en puerto 8004
INFO - Servidor de IO iniciado en puerto 8003
INFO - Presione ENTER para iniciar
```

**Después de presionar ENTER:**
```
INFO - Planificador de Largo plazo iniciado
INFO - Algoritmo de planificación: FIFO
```

### Paso 3: Iniciar CPU(s)
```bash
cd tp-2025-1c-nullpointers/cpu
./bin/cpu
```

**Logs esperados:**
```
INFO - CPU iniciada
INFO - Conectando al Kernel en puerto 8001
INFO - Conectando al Kernel en puerto 8004
INFO - Conectando a Memoria en puerto 8002
INFO - Conexiones establecidas
```

### Paso 4: Iniciar dispositivos IO
```bash
cd tp-2025-1c-nullpointers/io

# Terminal 1 - Dispositivo DISCO
./bin/io DISCO

# Terminal 2 - Dispositivo IMPRESORA
./bin/io IMPRESORA

# Terminal 3 - Dispositivo RED
./bin/io RED
```

**Logs esperados:**
```
INFO - Dispositivo IO 'DISCO' conectado al Kernel
INFO - Iniciando manejo de peticiones IO
```

## Handshakes y Protocolo

### 1. Handshake CPU ↔ Kernel
```
CPU → Kernel: HANDSHAKE (puerto 8001)
Kernel → CPU: HANDSHAKE_ACK

CPU → Kernel: HANDSHAKE (puerto 8004)
Kernel → CPU: HANDSHAKE_ACK
```

### 2. Handshake CPU ↔ Memoria
```
CPU → Memoria: HANDSHAKE (puerto 8002)
Memoria → CPU: HANDSHAKE_ACK
```

### 3. Handshake IO ↔ Kernel
```
IO → Kernel: HANDSHAKE_IO + nombre_dispositivo (puerto 8003)
Kernel → IO: HANDSHAKE_ACK
```

## Flujo de Comunicación

### 1. Inicialización de Proceso
```
Kernel → Memoria: INICIAR_PROCESO (pid, size)
Memoria → Kernel: EXITO/ERROR
Kernel → CPU: DEVOLVER_PROCESO (pid, pc, MOTIVO_SYSCALL)
```

### 2. Ejecución de Instrucción
```
CPU → Memoria: SOLICITAR_INSTRUCCION (pid, pc)
Memoria → CPU: instrucción
CPU → Kernel: DEVOLVER_PROCESO (pid, pc, MOTIVO_SYSCALL/EXIT/INTERRUPCION)
```

### 3. Operación IO
```
Kernel → IO: SOLICITUD_IO (pid, tiempo)
IO → Kernel: FIN_IO (pid)
```

## Pruebas del Sistema

### Prueba 1: Proceso Simple
1. Crear archivo `test.txt` con instrucciones:
```
NOOP
WRITE 10 HOLA
READ 10 5
EXIT
```

2. Ejecutar:
```bash
./bin/kernel test.txt 512
```

### Prueba 2: Múltiples CPUs
1. Iniciar 2-3 instancias de CPU en terminales separadas
2. Verificar que el Kernel distribuya procesos entre ellas

### Prueba 3: Dispositivos IO
1. Iniciar múltiples dispositivos IO
2. Ejecutar proceso con instrucciones IO:
```
IO DISCO 1000
IO IMPRESORA 500
EXIT
```

## Logs de Verificación

### Kernel
- `## (PID) Se crea el proceso - Estado: NEW`
- `## (PID) Pasa del estado NEW al estado READY`
- `## (PID) Pasa del estado READY al estado EXEC`

### CPU
- `## PID: X - FETCH - Program Counter: Y`
- `## PID: X - Ejecutando: INSTRUCCION - parametros`

### Memoria
- `Inicialización de proceso PID: X exitosa`
- `Lectura de instrucción en dirección: Y`

### IO
- `## PID: X - Inicio de IO - Tiempo: Y`
- `## PID: X - Fin de IO`

## Solución de Problemas

### Error: "Connection refused"
- Verificar que los puertos no estén ocupados
- Asegurar el orden correcto de inicio (Memoria → Kernel → CPU/IO)

### Error: "Address already in use"
```bash
# Encontrar proceso usando el puerto
netstat -tulpn | grep :8001

# Terminar proceso
kill -9 PID
```

### Error: "No such file or directory"
- Verificar que los ejecutables se compilaron correctamente
- Verificar rutas de archivos de configuración

## Comandos Útiles

### Verificar puertos en uso
```bash
netstat -tulpn | grep :800
```

### Ver logs en tiempo real
```bash
tail -f kernel.log
tail -f cpu.log
tail -f memoria.log
tail -f io.log
```

### Limpiar todos los procesos
```bash
pkill -f "cpu\|memoria\|kernel\|io"
```

## Archivos de Configuración

### kernel.config
```
IP_MEMORIA=127.0.0.1
PUERTO_MEMORIA=8002
PUERTO_ESCUCHA_DISPATCH=8001
PUERTO_ESCUCHA_INTERRUPT=8004
PUERTO_ESCUCHA_IO=8003
ALGORITMO_CORTO_PLAZO=FIFO
ALGORITMO_INGRESO_A_READY=FIFO
ALFA=0.5
ESTIMACION_INICIAL=10000
TIEMPO_SUSPENSION=4500
LOG_LEVEL=TRACE
```

### cpu.config
```
IP_MEMORIA=127.0.0.1
PUERTO_MEMORIA=8002
IP_KERNEL=127.0.0.1
PUERTO_KERNEL_DISPATCH=8001
PUERTO_KERNEL_INTERRUPT=8004
ENTRADAS_TLB=4
ENTRADAS_CACHE=8
REEMPLAZO_TLB=LRU
REEMPLAZO_CACHE=LRU
LOG_LEVEL=TRACE
```

### memoria.config
```
PUERTO_ESCUCHA=8002
TAMANIO_MEMORIA=4096
TAMANIO_PAGINA=64
ENTRADAS_POR_TABLA=4
NIVELES_PAGINACION=2
TAMANIO_SWAP=8192
LOG_LEVEL=TRACE
```

### io.config
```
IP_KERNEL=127.0.0.1
PUERTO_KERNEL=8003
LOG_LEVEL=TRACE
```
