#!/bin/bash

# Script para levantar todos los módulos del TP Final en orden
# Orden: memoria -> kernel -> cpu -> io

echo "=== Iniciando módulos del TP Final ==="

# Función para matar procesos en caso de error
cleanup() {
    echo ""
    echo "=== Limpiando procesos ==="
    pkill -f "./bin/memoria" 2>/dev/null
    pkill -f "./bin/kernel" 2>/dev/null  
    pkill -f "./bin/cpu" 2>/dev/null
    pkill -f "./bin/io" 2>/dev/null
    exit 1
}

# Configurar trap para limpiar en caso de interrupción
trap cleanup SIGINT SIGTERM

# Variables para almacenar PIDs de los procesos
MEMORIA_PID=""
KERNEL_PID=""
CPU_PID=""
IO_PID=""

echo ""
echo "=== PASO 1: Compilando todos los módulos ==="

# Compilar utils primero
echo "Compilando utils..."
cd /home/utnso/Desktop/tp-final-v2/utils
make clean all
if [ $? -ne 0 ]; then
    echo "Error compilando utils"
    exit 1
fi

# Compilar memoria
echo "Compilando memoria..."
cd /home/utnso/Desktop/tp-final-v2/memoria
make clean all
if [ $? -ne 0 ]; then
    echo "Error compilando memoria"
    exit 1
fi

# Compilar kernel
echo "Compilando kernel..."
cd /home/utnso/Desktop/tp-final-v2/kernel
make clean all
if [ $? -ne 0 ]; then
    echo "Error compilando kernel"
    exit 1
fi

# Compilar cpu
echo "Compilando cpu..."
cd /home/utnso/Desktop/tp-final-v2/cpu
make clean all
if [ $? -ne 0 ]; then
    echo "Error compilando cpu"
    exit 1
fi

# Compilar io
echo "Compilando io..."
cd /home/utnso/Desktop/tp-final-v2/io
make clean all
if [ $? -ne 0 ]; then
    echo "Error compilando io"
    exit 1
fi

echo "✅ Compilación completada exitosamente"

echo ""
echo "=== PASO 2: Iniciando módulos en orden ==="

# 1. Iniciar MEMORIA
echo "🚀 Iniciando MEMORIA..."
cd /home/utnso/Desktop/tp-final-v2/memoria
./bin/memoria &
MEMORIA_PID=$!
echo "   Memoria iniciada con PID: $MEMORIA_PID"
sleep 2

# 2. Iniciar KERNEL (requiere argumentos: archivo de pseudocódigo y tamaño)
echo "🚀 Iniciando KERNEL..."
cd /home/utnso/Desktop/tp-final-v2/kernel
# Nota: El kernel requiere argumentos. Usar valores por defecto o pedirlos al usuario.
echo "   El kernel requiere argumentos: <archivo_pseudocodigo> <tamaño_programa>"
echo "   Usando valores por defecto: process0 100"
./bin/kernel process0 100 &
KERNEL_PID=$!
echo "   Kernel iniciado con PID: $KERNEL_PID"
sleep 3

# 3. Iniciar CPU
echo "🚀 Iniciando CPU..."
cd /home/utnso/Desktop/tp-final-v2/cpu
./bin/cpu &
CPU_PID=$!
echo "   CPU iniciada con PID: $CPU_PID"
sleep 2

# 4. Iniciar IO
echo "🚀 Iniciando IO..."
cd /home/utnso/Desktop/tp-final-v2/io
./bin/io teclado &
IO_PID=$!
echo "   IO iniciado con PID: $IO_PID y device teclado"
sleep 1

echo ""
echo "=== ✅ TODOS LOS MÓDULOS INICIADOS EXITOSAMENTE ==="
echo ""
echo "PIDs de los procesos:"
echo "  - Memoria: $MEMORIA_PID"
echo "  - Kernel:  $KERNEL_PID"
echo "  - CPU:     $CPU_PID"
echo "  - IO:      $IO_PID"
echo ""
echo "Para detener todos los módulos, presiona Ctrl+C"
echo "O ejecuta: pkill -f 'bin/(memoria|kernel|cpu|io)'"

# Esperar a que terminen los procesos (mantener el script corriendo)
wait $MEMORIA_PID $KERNEL_PID $CPU_PID $IO_PID

echo ""
echo "=== Todos los procesos han terminado ==="
