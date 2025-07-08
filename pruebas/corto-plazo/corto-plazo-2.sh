#!/bin/bash
# Prueba Planificación Corto Plazo - SRT con un CPU
# Actividades:
# - Iniciar los módulos con SRT como algoritmo de corto plazo
# - Ejecutar con una instancia de CPU
# - Levantar IO Disco y esperar a que queden procesos infinitos, luego matar IO Disco

echo "=== Prueba de Planificación de Corto Plazo - SRT ==="
echo "Algoritmo: SRT con un CPU"

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
echo "Iniciando MEMORIA con configuración corto-plazo..."
cd /home/utnso/Desktop/tp-final-v2/memoria
cp /home/utnso/Desktop/tp-final-v2/config/corto-plazo/corto-plazo.config memoria.config
./bin/memoria &
MEMORIA_PID=$!
sleep 2

# 2. Iniciar KERNEL
echo "Iniciando KERNEL con configuración corto-plazo-2 (SRT)..."
cd /home/utnso/Desktop/tp-final-v2/kernel
cp /home/utnso/Desktop/tp-final-v2/config/corto-plazo/corto-plazo-2.config kernel.config
./bin/kernel PLANI_CORTO_PLAZO 0 &
KERNEL_PID=$!
sleep 3

# 3. Iniciar una instancia de CPU
echo "Iniciando CPU..."
cd /home/utnso/Desktop/tp-final-v2/cpu
cp /home/utnso/Desktop/tp-final-v2/config/corto-plazo/corto-plazo.config cpu.config
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
echo "=== Sistema iniciado con algoritmo SRT y 1 CPU ==="
echo "- Memoria PID: $MEMORIA_PID"
echo "- Kernel PID: $KERNEL_PID"
echo "- CPU PID: $CPU_PID"
echo "- IO Disco PID: $IO_PID"
echo ""
echo "Presiona ENTER en la terminal del kernel para comenzar la ejecución de procesos"
read

echo "Esperando a que queden sólo los procesos infinitos..."
echo "Presiona ENTER cuando desees matar el IO Disco"
read

# 5. Matar IO Disco
echo "Matando IO Disco..."
kill $IO_PID
echo "IO Disco terminado"

echo ""
echo "Prueba de SRT completada."
echo "Presiona ENTER para finalizar todos los procesos"
read

# Limpiar todos los procesos al finalizar
cleanup
