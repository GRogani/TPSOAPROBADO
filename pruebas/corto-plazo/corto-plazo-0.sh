#!/bin/bash
# Prueba Planificación Corto Plazo - FIFO con dos CPU
# Actividades:
# - Iniciar los módulos con FIFO como algoritmo de corto plazo
# - Ejecutar con dos instancias de CPU
# - Levantar un segundo IO Disco cuando se estén ejecutando los procesos infinitos
# - Verificar uso de ambos IO y luego matarlos

echo "=== Prueba de Planificación de Corto Plazo - FIFO ==="
echo "Algoritmo: FIFO con dos CPUs"

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
IO_PID_1=""
IO_PID_2=""

# Función para limpieza
cleanup() {
    echo ""
    echo "=== Limpiando procesos ==="
    kill $IO_PID_1 $IO_PID_2 $CPU_PID_1 $CPU_PID_2 $KERNEL_PID $MEMORIA_PID 2>/dev/null
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
echo "Iniciando KERNEL con configuración corto-plazo-0 (FIFO)..."
cd /home/utnso/Desktop/tp-final-v2/kernel
cp /home/utnso/Desktop/tp-final-v2/config/corto-plazo/corto-plazo-0.config kernel.config
./bin/kernel PLANI_CORTO_PLAZO 500 &
KERNEL_PID=$!
sleep 3

# 3. Iniciar dos instancias de CPU
echo "Iniciando primera instancia de CPU..."
cd /home/utnso/Desktop/tp-final-v2/cpu
cp /home/utnso/Desktop/tp-final-v2/config/corto-plazo/corto-plazo.config cpu.config
./bin/cpu &
CPU_PID_1=$!
sleep 2

echo "Iniciando segunda instancia de CPU..."
./bin/cpu &
CPU_PID_2=$!
sleep 2

# 4. Iniciar IO primario (disco)
echo "Iniciando IO Disco..."
cd /home/utnso/Desktop/tp-final-v2/io
./bin/io disco &
IO_PID_1=$!
sleep 1

echo ""
echo "=== Sistema iniciado con algoritmo FIFO y 2 CPUs ==="
echo "- Memoria PID: $MEMORIA_PID"
echo "- Kernel PID: $KERNEL_PID"
echo "- CPU1 PID: $CPU_PID_1"
echo "- CPU2 PID: $CPU_PID_2" 
echo "- IO Disco PID: $IO_PID_1"
echo ""
echo "Presiona ENTER en la terminal del kernel para comenzar la ejecución de procesos"
read
echo "- Memoria PID: $MEMORIA_PID"
echo "- Kernel PID: $KERNEL_PID"
echo "- CPU1 PID: $CPU_PID_1"
echo "- CPU2 PID: $CPU_PID_2" 
echo "- IO Disco PID: $IO_PID_1"
echo ""
echo "Esperando a que se estén ejecutando los procesos infinitos..."
echo "Presiona ENTER cuando desees iniciar el segundo IO Disco"
read

# 5. Iniciar segundo IO Disco
echo "Iniciando segundo IO Disco..."
cd /home/utnso/Desktop/tp-final-v2/io
./bin/io disco &
IO_PID_2=$!
echo "Segundo IO Disco iniciado con PID: $IO_PID_2"
echo ""

echo "Verificando el uso de ambos IO Disco..."
echo "Presiona ENTER cuando confirmes que ambos IO están siendo utilizados y desees matarlos"
read

# 6. Matar ambos IO Disco
echo "Matando ambos IO Disco..."
kill $IO_PID_1 $IO_PID_2
echo "IO Discos terminados"

echo ""
echo "Prueba de FIFO completada."
echo "Presiona ENTER para finalizar todos los procesos"
read

# Limpiar todos los procesos al finalizar
cleanup
