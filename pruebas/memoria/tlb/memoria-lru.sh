#!/bin/bash

# Directorio base del proyecto
BASE_DIR="/home/utnso/so-deploy/tp-2025-1c-Mi-Grupo-1234"
CONFIG_DIR="config/memoria/tlb/memoria.config"
CPU_CONFIG_DIR="config/memoria/tlb/memoria-lru.config"

# Compilar todos los módulos
echo "Compilando módulos..."

# Compilar módulo MEMORIA
echo "Compilando MEMORIA..."
cd ${BASE_DIR}/memoria
rm -f memoria.log
make clean all

# Compilar módulo KERNEL
echo "Compilando KERNEL..."
cd ${BASE_DIR}/kernel
rm -f kernel.log
make clean all

# Compilar módulo IO
echo "Compilando IO..."
cd ${BASE_DIR}/io
rm -f io.log
make clean all

# Compilar módulo CPU
echo "Compilando CPU..."
cd ${BASE_DIR}/cpu
rm -f cpu.log
make clean all

echo "Compilación completada. Iniciando módulos del sistema..."

# Iniciar módulo MEMORIA
cd ${BASE_DIR}/memoria
cp ${CONFIG_DIR} memoria.config
echo "Módulo MEMORIA configurado!"

# Esperamos unos segundos para que la memoria esté lista
sleep 1

# Iniciar módulo KERNEL
cd ${BASE_DIR}/kernel
cp ${CONFIG_DIR} kernel.config
echo "Módulo KERNEL configurado!"

sleep 1

# Configurar módulo cpu
cd ${BASE_DIR}/cpu
cp ${CPU_CONFIG_DIR} cpu.config
echo "Módulo CPU configurado!"

echo "Todos los módulos han sido iniciados."