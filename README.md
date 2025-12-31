# Heap Visualizer

Visualizador interactivo de estructuras de datos Montículo (Heap) y algoritmo Heapsort, desarrollado en C++ nativo utilizando la API de Windows (WinAPI) y GDI.

## Características

*   **Visualización en Tiempo Real**: Nodos y conexiones dibujados dinámicamente.
*   **Animaciones Suaves**: Interpolación de movimiento durante inserciones y ordenamiento.
*   **Modos de Operación**:
    *   **Max-Heap**: La raíz es el elemento mayor.
    *   **Min-Heap**: La raíz es el elemento menor.
    *   **Heapsort**: Visualización del algoritmo de ordenamiento por extracción.
*   **Tecnología**: C++17, WinAPI pura, GDI (Double Buffering).

## Requisitos

*   Sistema Operativo: Windows 10/11.
*   Compilador C++: MinGW (g++) recomendado.

## Compilación

### Opción 1: Línea de Comandos (MinGW/G++)
Abre una terminal en la carpeta del proyecto y ejecuta:

```bash
g++ main.cpp -o heapviz.exe -lgdi32 -luser32 -mwindows -static
```

### Opción 2: Code::Blocks IDE
1.  Crea un **Empty Project** y añade `main.cpp`.
2.  Ve a **Project -> Build options**.
3.  En la pestaña **Compiler settings > Other compiler options**, añade:
    ```
    -municode
    ```
4.  En la pestaña **Linker settings > Link libraries**, añade:
    ```
    gdi32
    user32
    ```
5.  En **Linker settings > Other linker options**, añade:
    ```
    -mwindows
    ```

## Uso

1.  **Insertar**: Escribe un número en la caja de texto superior izquierda y pulsa "Insert Max" o "Insert Min".
2.  **Ordenar**: Con varios elementos en pantalla, pulsa "Heapsort" para ver el proceso de ordenamiento.
3.  **Reiniciar**: Pulsa "Reset" para limpiar la pantalla.

## Autor
Desarrollado para el curso de Estructuras de Datos Avanzadas.
