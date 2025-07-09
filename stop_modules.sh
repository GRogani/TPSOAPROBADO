#!/bin/bash

# Script para detener todos los módulos del TP Final

echo "=== Deteniendo módulos del TP Final ==="

echo "Deteniendo IO..."
pkill -f "./bin/io" 2>/dev/null
sleep 1

echo "Deteniendo CPU..."
pkill -f "./bin/cpu" 2>/dev/null
sleep 1

echo "Deteniendo Kernel..."
pkill -f "./bin/kernel" 2>/dev/null
sleep 1

echo "Deteniendo Memoria..."
pkill -f "./bin/memoria" 2>/dev/null
sleep 1

echo "✅ Todos los módulos detenidos"

# Verificar si quedan procesos
REMAINING=$(pgrep -f "bin/(memoria|kernel|cpu|io)" | wc -l)
if [ $REMAINING -gt 0 ]; then
    echo "⚠️  Advertencia: Algunos procesos podrían seguir ejecutándose"
    echo "Ejecuta 'pkill -9 -f \"bin/(memoria|kernel|cpu|io)\"' para forzar la terminación"
else
    echo "✅ Limpieza completa"
fi
