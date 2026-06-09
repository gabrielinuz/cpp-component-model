#!/bin/bash
# Forzar que el script termine si algún comando falla
set -e

# Asegurar la existencia del directorio de destino para los módulos
mkdir -p lib

# 1. Compilar el Componente como una Biblioteca Compartida (Dynamic Shared Object)
# Agregamos -pthread porque HttpServerComponent utiliza hilos internamente.
g++ -std=c++17 -c -fPIC src/http_server_component.cpp -o http_server_component.o
g++ -std=c++17 -shared -pthread -o lib/http_server.so http_server_component.o

# 2. Compilar el Ejecutable Principal (Host)
# Enlazamos -ldl para dlopen y -pthread para dar soporte al entorno multihilo del componente cargado.
g++ -std=c++17 main.cpp -o host_app.bin -ldl -pthread

# 3. Ejecutar la aplicación
./host_app.bin