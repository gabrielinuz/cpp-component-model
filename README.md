# Modelo de Componente Concurrente en C++

Este repositorio contiene la especificación y estructura base para el modelo de componentes concurrentes.

## Propuesta de Nuevo Componente: NetworkTelemetryComponent

### 1. Descripción General

El `NetworkTelemetryComponent` es un componente concurrente diseñado para recopilar, procesar y reportar métricas de red en tiempo real (como latencia, pérdida de paquetes y ancho de banda utilizado). Opera de forma asincrónica para evitar bloquear los hilos principales de la aplicación o del procesamiento de datos de otros componentes.

### 2. Interfaz del Componente (API Pública)

* `void startMonitoring()`: Inicia el hilo secundario de recolección de métricas.
* `void stopMonitoring()`: Detiene la recolección de métricas de forma segura.
* `TelemetryData getLatestMetrics()`: Devuelve de manera segura (thread-safe) la última captura de métricas del sistema.

### 3. Mecanismos de Concurrencia y Sincronización

* **Hilo de Trabajo Dedicado:** Utiliza un `std::thread` interno que se encarga de realizar los pings y mediciones de red en segundo plano a intervalos regulares.
* **Protección de Datos:** Las métricas se almacenan en una estructura interna protegida por un `std::mutex`. Se emplea `std::lock_guard` para garantizar la exclusión mutua cuando el hilo de trabajo escribe los datos o cuando la aplicación los lee mediante `getLatestMetrics()`.
* **Control de Ciclo de Vida:** Se utiliza un flag atómico (`std::atomic<bool>`) para controlar de manera segura cuándo debe finalizar el bucle del hilo secundario al llamar a `stopMonitoring()`.
