#!/bin/bash

echo "=== SISTEMA OPERATIVO SIMULADO - PRUEBA COMPLETA ==="
echo ""

# Colores para output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Función para imprimir con colores
print_status() {
    echo -e "${GREEN}[✓]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[!]${NC} $1"
}

print_error() {
    echo -e "${RED}[✗]${NC} $1"
}

# Función para limpiar procesos
cleanup() {
    echo ""
    print_warning "Limpiando procesos..."
    pkill -f "cpu\|memoria\|kernel\|io" 2>/dev/null
    sleep 2
}

# Trap para limpiar al salir
trap cleanup EXIT

# Verificar que estamos en el directorio correcto
if [ ! -d "cpu" ] || [ ! -d "memoria" ] || [ ! -d "kernel" ] || [ ! -d "io" ]; then
    print_error "Debe ejecutar este script desde la raíz del proyecto (tp-2025-1c-nullpointers/)"
    exit 1
fi

# Paso 1: Compilar todos los módulos
echo "1. COMPILANDO MÓDULOS..."
echo ""

# Compilar CPU
echo "Compilando CPU..."
cd cpu
make clean > /dev/null 2>&1
if make > /dev/null 2>&1; then
    print_status "CPU compilada correctamente"
else
    print_error "Error compilando CPU"
    exit 1
fi
cd ..

# Compilar Memoria
echo "Compilando Memoria..."
cd memoria
make clean > /dev/null 2>&1
if make > /dev/null 2>&1; then
    print_status "Memoria compilada correctamente"
else
    print_error "Error compilando Memoria"
    exit 1
fi
cd ..

# Compilar Kernel
echo "Compilando Kernel..."
cd kernel
make clean > /dev/null 2>&1
if make > /dev/null 2>&1; then
    print_status "Kernel compilado correctamente"
else
    print_error "Error compilando Kernel"
    exit 1
fi
cd ..

# Compilar IO
echo "Compilando IO..."
cd io
make clean > /dev/null 2>&1
if make > /dev/null 2>&1; then
    print_status "IO compilado correctamente"
else
    print_error "Error compilando IO"
    exit 1
fi
cd ..

echo ""
print_status "Todos los módulos compilados correctamente"
echo ""

# Paso 2: Crear archivo de prueba
echo "2. CREANDO ARCHIVO DE PRUEBA..."
cat > test_proceso.txt << EOF
NOOP
WRITE 10 HOLA_MUNDO
READ 10 12
IO DISCO 1000
WRITE 20 TEST
READ 20 4
EXIT
EOF
print_status "Archivo de prueba creado: test_proceso.txt"
echo ""

# Paso 3: Verificar puertos disponibles
echo "3. VERIFICANDO PUERTOS..."
for port in 8001 8002 8003 8004; do
    if netstat -tuln 2>/dev/null | grep -q ":$port "; then
        print_warning "Puerto $port está en uso"
    else
        print_status "Puerto $port disponible"
    fi
done
echo ""

# Paso 4: Iniciar Memoria
echo "4. INICIANDO MEMORIA..."
cd memoria
./bin/memoria > memoria.log 2>&1 &
MEMORIA_PID=$!
sleep 2

if kill -0 $MEMORIA_PID 2>/dev/null; then
    print_status "Memoria iniciada (PID: $MEMORIA_PID)"
else
    print_error "Error iniciando Memoria"
    exit 1
fi
cd ..
echo ""

# Paso 5: Iniciar Kernel
echo "5. INICIANDO KERNEL..."
cd kernel
./bin/kernel ../test_proceso.txt 1024 > kernel.log 2>&1 &
KERNEL_PID=$!
sleep 3

if kill -0 $KERNEL_PID 2>/dev/null; then
    print_status "Kernel iniciado (PID: $KERNEL_PID)"
    print_warning "Presiona ENTER en la terminal del Kernel para continuar"
else
    print_error "Error iniciando Kernel"
    exit 1
fi
cd ..
echo ""

# Paso 6: Iniciar CPU
echo "6. INICIANDO CPU..."
cd cpu
./bin/cpu > cpu.log 2>&1 &
CPU_PID=$!
sleep 2

if kill -0 $CPU_PID 2>/dev/null; then
    print_status "CPU iniciada (PID: $CPU_PID)"
else
    print_error "Error iniciando CPU"
    exit 1
fi
cd ..
echo ""

# Paso 7: Iniciar dispositivos IO
echo "7. INICIANDO DISPOSITIVOS IO..."
cd io

# Dispositivo DISCO
./bin/io DISCO > io_disco.log 2>&1 &
IO_DISCO_PID=$!
sleep 1

# Dispositivo IMPRESORA
./bin/io IMPRESORA > io_impresora.log 2>&1 &
IO_IMPRESORA_PID=$!
sleep 1

if kill -0 $IO_DISCO_PID 2>/dev/null && kill -0 $IO_IMPRESORA_PID 2>/dev/null; then
    print_status "Dispositivos IO iniciados (DISCO: $IO_DISCO_PID, IMPRESORA: $IO_IMPRESORA_PID)"
else
    print_error "Error iniciando dispositivos IO"
    exit 1
fi
cd ..
echo ""

# Paso 8: Monitorear logs
echo "8. MONITOREANDO SISTEMA..."
echo "Logs disponibles:"
echo "  - Memoria: tail -f memoria/memoria.log"
echo "  - Kernel:  tail -f kernel/kernel.log"
echo "  - CPU:     tail -f cpu/cpu.log"
echo "  - IO DISCO: tail -f io/io_disco.log"
echo "  - IO IMPRESORA: tail -f io/io_impresora.log"
echo ""

print_status "Sistema iniciado correctamente!"
echo ""
echo "Para detener el sistema, presiona Ctrl+C"
echo ""

# Esperar señal de interrupción
while true; do
    sleep 1
    # Verificar que todos los procesos estén vivos
    if ! kill -0 $MEMORIA_PID 2>/dev/null; then
        print_error "Memoria se detuvo inesperadamente"
        break
    fi
    if ! kill -0 $KERNEL_PID 2>/dev/null; then
        print_error "Kernel se detuvo inesperadamente"
        break
    fi
    if ! kill -0 $CPU_PID 2>/dev/null; then
        print_error "CPU se detuvo inesperadamente"
        break
    fi
done

echo ""
print_warning "Prueba completada" 