#!/bin/bash

echo "=== INICIANDO SISTEMA OPERATIVO SIMULADO ==="
echo ""

# Colores
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo -e "${GREEN}1. Compilando módulos...${NC}"
echo ""

# Compilar todos los módulos
for modulo in cpu memoria kernel io; do
    echo "Compilando $modulo..."
    cd $modulo
    make clean > /dev/null 2>&1
    make > /dev/null 2>&1
    if [ $? -eq 0 ]; then
        echo "✓ $modulo compilado"
    else
        echo "✗ Error compilando $modulo"
        exit 1
    fi
    cd ..
done

echo ""
echo -e "${GREEN}2. Iniciando sistema...${NC}"
echo ""

# Crear archivo de proceso de ejemplo si no existe
if [ ! -f "ejemplo_proceso.txt" ]; then
    echo "Creando archivo de ejemplo..."
    cat > ejemplo_proceso.txt << EOF
NOOP
WRITE 10 HOLA
READ 10 4
IO DISCO 1000
EXIT
EOF
fi

echo "Orden de inicio:"
echo "1. Memoria (puerto 8002)"
echo "2. Kernel (puertos 8001, 8003, 8004)"
echo "3. CPU (se conecta a Kernel y Memoria)"
echo "4. IO (se conecta a Kernel)"
echo ""

echo -e "${YELLOW}Instrucciones:${NC}"
echo "1. Abre 4 terminales nuevas"
echo "2. En cada terminal, ejecuta los siguientes comandos:"
echo ""
echo "Terminal 1 (Memoria):"
echo "  cd memoria && ./bin/memoria"
echo ""
echo "Terminal 2 (Kernel):"
echo "  cd kernel && ./bin/kernel ../ejemplo_proceso.txt 1024"
echo "  (Presiona ENTER cuando aparezca el mensaje)"
echo ""
echo "Terminal 3 (CPU):"
echo "  cd cpu && ./bin/cpu"
echo ""
echo "Terminal 4 (IO):"
echo "  cd io && ./bin/io DISCO"
echo ""
echo -e "${YELLOW}Para detener el sistema:${NC}"
echo "  pkill -f 'cpu\|memoria\|kernel\|io'"
echo ""
echo -e "${YELLOW}Para ver logs:${NC}"
echo "  tail -f memoria/memoria.log"
echo "  tail -f kernel/kernel.log"
echo "  tail -f cpu/cpu.log"
echo "  tail -f io/io.log"
echo ""

echo -e "${GREEN}Sistema listo para iniciar!${NC}" 