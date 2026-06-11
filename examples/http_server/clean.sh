#!/bin/bash

# Forzar que el script termine si algún comando falla
set -e

# Función para borrar si existe
borrar_si_existe() {
    local ruta="$1"
    if [ -f "$ruta" ]; then
        rm -v "$ruta"
    else
        echo "El archivo no existe o no es válido: $ruta"
    fi
}

# Limpieza de bibliotecas
borrar_si_existe  "./lib/http_server.so"

# Limpieza de aplicación
borrar_si_existe  "host.app"