# Shell Personalizado

Este proyecto implementa un shell personalizado en C que permite a los usuarios ejecutar comandos en la terminal con funcionalidades adicionales.

## Características

- Interfaz de línea de comandos interactiva
- Manejo de señales (SIGINT)
- Animaciones de bienvenida y despedida
- Autocompletado de comandos con corrección de errores
- Soporte para comandos en segundo plano
- Búsqueda de comandos similares cuando se ingresa un comando incorrecto

## Estructura del Proyecto

- `shell.c`: Archivo principal que contiene la implementación del shell
- `ascii-frames.json`: Archivo con frames de animación ASCII
- `ascii-frames-end.json`: Archivo con frames de animación ASCII para la despedida
- `fibonacci.py`: Script de Python para cálculos de Fibonacci
- `dwimsh`: Ejecutable del shell

## Requisitos

- Compilador GCC
- Sistema operativo Unix/Linux
- Python 3 (para el script de Fibonacci)

## Compilación

Para compilar el proyecto, ejecute:

```bash
gcc shell.c -o dwimsh
```

## Uso

1. Ejecute el shell:
```bash
./dwimsh
```

2. Ingrese comandos como lo haría en una terminal normal
3. Para salir, escriba `exit` o presione Ctrl+C

## Funcionalidades Especiales

- El shell muestra una animación de bienvenida al iniciar
- Si ingresa un comando incorrecto, el shell sugerirá el comando más similar
- Soporte para ejecutar comandos en segundo plano usando `&`
- Animación de despedida al cerrar el shell

## Autor

- Alberth Godoy (12111345)

## Licencia

Este proyecto es parte de un trabajo académico para la Universidad Unitec. 