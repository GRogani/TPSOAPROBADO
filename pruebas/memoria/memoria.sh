#!/bin/bash
# Prueba de Memoria
# Actividades:
# - Iniciar los módulos
# - Ejecutar procesos que quedarán en estado SUSP. BLOCKED

echo "=== Prueba de Memoria ==="

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
IO_PID=""

# Función para limpieza
cleanup() {
    echo ""
    echo "=== Limpiando procesos ==="
    kill $IO_PID $CPU_PID_1 $CPU_PID_2 $CPU_PID_3 $KERNEL_PID $MEMORIA_PID 2>/dev/null
    exit 1
}

trap cleanup SIGINT SIGTERM

# 1. Iniciar MEMORIA
echo "Iniciando MEMORIA con configuración memoria..."
cd /home/utnso/Desktop/tp-final-v2/memoria
cp /home/utnso/Desktop/tp-final-v2/config/memoria/memoria.config memoria.config
./bin/memoria &
MEMORIA_PID=$!
sleep 2

# 2. Iniciar KERNEL
echo "Iniciando KERNEL con configuración memoria..."
cd /home/utnso/Desktop/tp-final-v2/kernel
cp /home/utnso/Desktop/tp-final-v2/config/memoria/memoria.config kernel.config
./bin/kernel MEMORIA 0 &
KERNEL_PID=$!
sleep 3

# 3. Iniciar tres instancias de CPU con diferentes configuraciones
echo "Iniciando primera instancia de CPU..."
cd /home/utnso/Desktop/tp-final-v2/cpu
cp /home/utnso/Desktop/tp-final-v2/config/memoria/memoria-0.config cpu.config
./bin/cpu &
CPU_PID_1=$!
sleep 1

echo "Iniciando segunda instancia de CPU..."
cp /home/utnso/Desktop/tp-final-v2/config/memoria/memoria-1.config cpu.config
./bin/cpu &
CPU_PID_2=$!
sleep 1

echo "Iniciando tercera instancia de CPU..."
cp /home/utnso/Desktop/tp-final-v2/config/memoria/memoria-2.config cpu.config
./bin/cpu &
CPU_PID_3=$!
sleep 1

# 4. Iniciar IO Disco
echo "Iniciando IO Disco..."
cd /home/utnso/Desktop/tp-final-v2/io
./bin/io disco &
IO_PID=$!
sleep 1

echo ""
echo "=== Sistema iniciado para prueba de memoria ==="
echo "- Memoria PID: $MEMORIA_PID"
echo "- Kernel PID: $KERNEL_PID"
echo "- CPU1 PID: $CPU_PID_1"
echo "- CPU2 PID: $CPU_PID_2"
echo "- CPU3 PID: $CPU_PID_3"
echo "- IO Disco PID: $IO_PID"
echo ""
echo "Presiona ENTER en la terminal del kernel para comenzar la ejecución de procesos"
read

echo "Esperando a que los procesos creados se queden en estado SUSP. BLOCKED..."
echo "Presiona ENTER cuando hayas verificado que los procesos están en estado SUSP. BLOCKED"
read

echo ""
echo "Prueba de memoria completada."
echo "Presiona ENTER para finalizar todos los procesos"
read

# Limpiar todos los procesos al finalizar
cleanup

