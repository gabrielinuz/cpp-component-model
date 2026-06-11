#!/bin/bash
# Forzar que el script termine si algún comando falla
set -e

# Asegurar la existencia del directorio de destino para los módulos
mkdir -p lib

# 1. Compilar el objeto
g++ -std=c++17 -c -fPIC src/sqlite_component.cpp -o sqlite_component.o

# 2. Enlazar la biblioteca compartida (NOTA: -lsqlite3 va al final)
##@NOTE: 
##  En GCC, el orden de las banderas importa críticamente. 
##  El enlazador procesa los archivos de izquierda a derecha. 
##  Si pones -lsqlite3 antes del archivo objeto (sqlite_component.o), 
##  el enlazador revisa la biblioteca SQLite, ve que en ese momento 
##  exacto nadie ha solicitado ninguna función de ella (porque aún 
##  no ha leído tu archivo objeto), y la descarta de la memoria. 
##  Luego lee tu objeto, encuentra llamadas a sqlite3_open, y falla 
##  porque ya descartó la biblioteca.
g++ -std=c++17 -shared -o lib/sqlite_handler.so sqlite_component.o -lsqlite3

# 3. Compilar el Ejecutable Principal
g++ -std=c++17 main.cpp -o host_app.bin -ldl -pthread

# 4. Ejecutar
./host_app.bin