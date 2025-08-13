# 🚦 Mini Mini Proyecto – Simulación de Tráfico con OpenMP

## 📌 Descripción
Este proyecto simula el comportamiento de una intersección con múltiples semáforos y vehículos.  
Cada vehículo está asignado a un semáforo y su movimiento depende del estado del mismo:
- **VERDE** → Avanza a su velocidad asignada.
- **AMARILLO** → Avanza 1 unidad por iteración.
- **ROJO** → Se detiene.

Se implementan dos versiones:
1. **Secuencial** → Todo el procesamiento se realiza en un solo hilo.
2. **Paralela** → Uso de **OpenMP** para acelerar la simulación dividiendo tareas en múltiples hilos.

---

---

## ⚙️ Requisitos
- **Compilador C** compatible (`gcc`, `clang`, `gcc-XX` de Homebrew en macOS).
- Para la versión paralela:
  - **OpenMP** instalado.
  - En macOS con Homebrew:
    ```bash
    brew install gcc
    ```
  - En Linux:
    ```bash
    sudo apt install gcc
    ```

---

## 🖥️ Compilación

### 1. Versión Secuencial
```bash
gcc simulacion_secuencial.c -o simulacion_secuencial
