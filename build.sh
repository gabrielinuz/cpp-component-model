# 1. Compilar el Componente como una Biblioteca Compartida (Dynamic Shared Object)
# Usamos -fPIC (Position Independent Code) vital para bibliotecas compartidas en Linux
g++ -std=c++17 -c -fPIC src/GreeterComponent.cpp -o GreeterComponent.o
g++ -std=c++17 -shared -o lib/Greeter.so GreeterComponent.o

# 2. Compilar el Ejecutable Principal
# Necesitamos enlazar la biblioteca -ldl para poder usar dlopen, dlclose, dlsym en Linux
g++ -std=c++17 main.cpp -o HostApp.bin -ldl

# 3. Ejecutar la aplicación
./HostApp.bin