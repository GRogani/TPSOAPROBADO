#!/bin/bash
# Prueba de Estabilidad General
# Actividades:
# - Iniciar los módulos con 4 CPUs
# - Dejar correr el sistema por un tiempo prolongado

echo "=== Prueba de Estabilidad General ==="

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
CPU_PID_1=""
CPU_PID_2=""
CPU_PID_3=""
CPU_PID_4=""
IO_PID=""

# Función para limpieza
cleanup() {
    echo ""
    echo "=== Limpiando procesos ==="
    kill $IO_PID $CPU_PID_1 $CPU_PID_2 $CPU_PID_3 $CPU_PID_4 $KERNEL_PID $MEMORIA_PID 2>/dev/null
    exit 1
}

trap cleanup SIGINT SIGTERM

# 1. Iniciar MEMORIA
echo "Iniciando MEMORIA con configuración estabilidad-general..."
cd /home/utnso/Desktop/tp-final-v2/memoria
cp /home/utnso/Desktop/tp-final-v2/config/estabilidad-general/estabilidad-general.config memoria.config
./bin/memoria &
MEMORIA_PID=$!
sleep 2

# 2. Iniciar KERNEL
echo "Iniciando KERNEL con configuración estabilidad-general..."
cd /home/utnso/Desktop/tp-final-v2/kernel
cp /home/utnso/Desktop/tp-final-v2/config/estabilidad-general/estabilidad-general.config kernel.config
./bin/kernel ESTABILIDAD_GENERAL 0 &
KERNEL_PID=$!
sleep 3

# 3. Iniciar cuatro instancias de CPU con diferentes configuraciones
echo "Iniciando primera instancia de CPU..."
cd /home/utnso/Desktop/tp-final-v2/cpu
cp /home/utnso/Desktop/tp-final-v2/config/estabilidad-general/estabilidad-general-0.config cpu.config
./bin/cpu &
CPU_PID_1=$!
sleep 1

echo "Iniciando segunda instancia de CPU..."
cp /home/utnso/Desktop/tp-final-v2/config/estabilidad-general/estabilidad-general-1.config cpu.config
./bin/cpu &
CPU_PID_2=$!
sleep 1

echo "Iniciando tercera instancia de CPU..."
cp /home/utnso/Desktop/tp-final-v2/config/estabilidad-general/estabilidad-general-2.config cpu.config
./bin/cpu &
CPU_PID_3=$!
sleep 1

echo "Iniciando cuarta instancia de CPU..."
cp /home/utnso/Desktop/tp-final-v2/config/estabilidad-general/estabilidad-general-3.config cpu.config
./bin/cpu &
CPU_PID_4=$!
sleep 1

# 4. Iniciar IO Disco
echo "Iniciando IO Disco..."
cd /home/utnso/Desktop/tp-final-v2/io
./bin/io disco &
IO_PID=$!
sleep 1

echo ""
echo "=== Sistema iniciado para prueba de estabilidad general ==="
echo "- Memoria PID: $MEMORIA_PID"
echo "- Kernel PID: $KERNEL_PID"
echo "- CPU1 PID: $CPU_PID_1"
echo "- CPU2 PID: $CPU_PID_2"
echo "- CPU3 PID: $CPU_PID_3"
echo "- CPU4 PID: $CPU_PID_4"
echo "- IO Disco PID: $IO_PID"
echo ""
echo "Presiona ENTER en la terminal del kernel para comenzar la ejecución de procesos"
read

echo "El sistema se está ejecutando... Presiona ENTER para finalizar la prueba cuando lo desees"
echo "Dejando el sistema correr por un tiempo prolongado para verificar estabilidad"
read

echo ""
echo "Prueba de estabilidad general completada."
echo "Presiona ENTER para finalizar todos los procesos"
read

# Limpiar todos los procesos al finalizar
cleanup



