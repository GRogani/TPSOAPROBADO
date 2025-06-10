#!/bin/bash

# Script simple para levantar módulos del TP Final
# Uso: ./start_simple.sh [archivo_pseudocodigo] [tamaño_programa]

echo "=== Iniciando módulos del TP Final (versión simple) ==="

# Parámetros para el kernel (con valores por defecto)
ARCHIVO_PSEUDO=${1:-"process0"}
TAMANO_PROGRAMA=${2:-"100"}

echo "Parámetros del kernel: archivo=$ARCHIVO_PSEUDO, tamaño=$TAMANO_PROGRAMA"
echo ""

# 1. Memoria
echo "🚀 Iniciando MEMORIA..."
cd /home/utnso/Desktop/tp-final-v2/memoria && ./bin/memoria &
sleep 2

# 2. Kernel 
echo "🚀 Iniciando KERNEL..."
cd /home/utnso/Desktop/tp-final-v2/kernel && ./bin/kernel $ARCHIVO_PSEUDO $TAMANO_PROGRAMA &
sleep 3

# 3. CPU
echo "🚀 Iniciando CPU..."
cd /home/utnso/Desktop/tp-final-v2/cpu && ./bin/cpu &
sleep 2

# 4. IO
echo "🚀 Iniciando IO..."
cd /home/utnso/Desktop/tp-final-v2/io && ./bin/io &
sleep 1

echo ""
echo "✅ Todos los módulos iniciados"
echo "Para detener: pkill -f 'bin/(memoria|kernel|cpu|io)'"

# Mantener el script corriendo
wait
