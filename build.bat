::1. Compilar el Componente como una Biblioteca Compartida (Dynamic Shared Object)
g++ -c src\GreeterComponent.cpp -o GreeterComponent.o
g++ -shared -o libGreeter.dll GreeterComponent.o

::2. Compilar el Ejecutable Principal
::Necesitamos enlazar la biblioteca -ldl para poder usar dlopen, dlclose, dlsym en Linux
g++ main.cpp -o hostApp -ldl

::3. Ejecutar la aplicación
hostApp.exe