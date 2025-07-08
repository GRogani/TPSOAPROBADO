#!/bin/bash
# Prueba Planificación Mediano/Largo Plazo - Algoritmo por defecto
# Actividades:
# - Iniciar los módulos
# - Ejecutar procesos con algoritmo por defecto
# - Esperar la finalización normal

echo "=== Prueba de Planificación de Mediano/Largo Plazo - Por Defecto ==="

# Compilar todos los módulos primero
echo "Compilando módulos..."
cd /home/utnso/Desktop/tp-final-v2/utils && make clean all
cd /home/utnso/Desktop/tp-final-v2/memoria && make clean all
cd /home/utnso/Desktop/tp-final-v2/kernel && make clean all
cd /home/utnso/Desktop/tp-final-v2/cpu && make clean all
cd /home/utnso/Desktop/tp-final-v2/io && make clean all

# PIDs para gestionar procesos
MEMORIA_PID=""
KERNEL_PID=""
CPU_PID=""
IO_PID=""

# Función para limpieza
cleanup() {
    echo ""
    echo "=== Limpiando procesos ==="
    kill $IO_PID $CPU_PID $KERNEL_PID $MEMORIA_PID 2>/dev/null
    exit 1
}

trap cleanup SIGINT SIGTERM

# 1. Iniciar MEMORIA
echo "Iniciando MEMORIA con configuración mediano-largo-plazo..."
cd /home/utnso/Desktop/tp-final-v2/memoria
cp /home/utnso/Desktop/tp-final-v2/config/mediano-largo-plazo/mediano-largo-plazo.config memoria.config
./bin/memoria &
MEMORIA_PID=$!
sleep 2

# 2. Iniciar KERNEL
echo "Iniciando KERNEL con configuración mediano-largo-plazo-0 (default)..."
cd /home/utnso/Desktop/tp-final-v2/kernel
cp /home/utnso/Desktop/tp-final-v2/config/mediano-largo-plazo/mediano-largo-plazo-0.config kernel.config
./bin/kernel PLANI_LYM_PLAZO 0 &
KERNEL_PID=$!
sleep 3

# 3. Iniciar CPU
echo "Iniciando CPU..."
cd /home/utnso/Desktop/tp-final-v2/cpu
cp /home/utnso/Desktop/tp-final-v2/config/mediano-largo-plazo/mediano-largo-plazo.config cpu.config
./bin/cpu &
CPU_PID=$!
sleep 2

# 4. Iniciar IO Disco
echo "Iniciando IO Disco..."
cd /home/utnso/Desktop/tp-final-v2/io
./bin/io disco &
IO_PID=$!
sleep 1

echo ""
echo "=== Sistema iniciado con algoritmo de Mediano/Largo plazo por defecto ==="
echo "- Memoria PID: $MEMORIA_PID"
echo "- Kernel PID: $KERNEL_PID"
echo "- CPU PID: $CPU_PID"
echo "- IO Disco PID: $IO_PID"
echo ""
echo "Presiona ENTER en la terminal del kernel para comenzar la ejecución de procesos"
read

echo "Esperando la finalización de procesos con normalidad..."
echo "Presiona ENTER cuando los procesos hayan finalizado para terminar la prueba"
read

echo ""
echo "Prueba de planificación Mediano/Largo plazo por defecto completada."
echo "Presiona ENTER para finalizar todos los procesos"
read

# Limpiar todos los procesos al finalizar
cleanup

