# Explicación de la estructura del lab3

## Introducción
Te explico un poco como fue el proceso que fui siguiendo para mejorar el rendimiento del programa.
Partí de la versión de referencia del lab 2 (versión 0), intenté paralelizar el programa directamente a partir de ese código (versión 7) y lo pude hacer, pero no me sentía satisfecho con la generación de números aleatorios, así que me puse a mejorar el rendimiento de esa parte, entré en un ciclo de locura, errores, correcciones y mejoras (intentaba nunca perder lo anterior y poder comparar los cambios, por eso terminé con un montón de versiones intermedias que decidí dejarlas porque capaz sirven para sacar ideas o retroceder en alguna parte del código y quitar cambios que hice y que ahora nos vuelven el código ineficiente) y finalmente llegué a la versión 5, que es la mejor (creo) sin usar OpenMP. A partir de esa versión probé paralelizar el programa, tuve un par de problemas haciendo independiente el generador de números aleatorios para cada hilo, pero al final logré algo que CREO que funciona bien.

## Directorios
### 0_baseline
Es la versión de referencia del laboratorio 2, la que se usó para grabar el video.
Resultado: El mismo que antes, aprox 140~145 spins/ms.

### 1_baseline_mejorada
Se cambiaron de la linea 63 a 74 de ising.c y el método de calculo de probabilidad. 
Los cambios fueron:
- Ahora el xoshiro256plus devuelve un vector con 8 valores float aleatorios.
- Ahora la carga de "memoria -> vector" de los exponenciales es más eficiente.

Resultado: Mejora de 1.5x, aprox 210 spins/ms.

### 2_baseline_mejorada
Ahora los exponenciales que se precalculan se hacen de manera vectorizada, usando una función que calcula 8 exponenciales a la vez.

Resultado: No mejora casi nada con respecto a la versión anterior, pero lo dejé porque tampoco empeora nada.

### 3_baseline_mejorada
Agregué un borde de 1 celda alrededor de la matriz, para evitar tener que usar módulo. Esos valores se actualizan constantemente para que el funcionamiento sea equivalente al anterior.

Resultado: Mejora un toque, creo que llega a 215 spins/ms.

### 4_baseline_mejorada
Ahora se precalculan todos los exponenciales que se van a usar al principio del programa.

Resultado: No mejora casi nada con respecto a la versión anterior, pero lo dejé porque tampoco empeora nada.


### 5_baseline_mejorada
Mejora al xoshiro, usando funciones de intrinsics para todo el computo de aleatorios.

Resultado: Mejora un poco, creo que llega a 220 spins/ms.

### 6_omp
Acá apliqué OpenMP para paralelizar el programa, lo usé solo 1 vez en la función update.
Se usa la directiva #pragma omp parallel for.
También se aplican varios cambios al generador de números aleatorios, para que sea independiente en cada hilo.

Resultado: Mejora bastante, dependiendo de la cantidad de hilos. Logré llegar a 1700 spins/ms con 24 hilos, pero no sé si es el límite. Usando 1 hilo, el rendimiento es más o menos 200 spins/ms, no agrega tanto overhead el openmp.

### 7_omp_sin_mejoras
Esta es la versión de OpenMP sin aplicar ningún cambio de las "baseline_mejorada", para probar la diferencia de rendimiento.

Resultado: El rendimiento es similar al de la versión 6_omp, logré llegar a 1700 spins/ms con 24 hilos, pero no sé si es el límite. Usando 1 hilo, el rendimiento es más o menos 130~135 spins/ms, no agrega tanto overhead el openmp (recordar que el rendimiento de la baseline era 140~145 spins/ms).

## Conclusiones

- A partir de cierto número de hilos, el rendimiento deja de mejorar y empieza a bajar, si ejecutas el programa sin límite (usa los 48 procesadores del server Atom) el rendimiento no es óptimo.
- El cuello de botella en este momento CREO que es esta función: _mm256_i32gather_ps, que se encarga de cargar los valores del exponencial desde la memoria al registro vectorizado.

## Notas

- La demo capaz esta rota en algunas versiones, me aseguré de que funcione en la versión 6_omp, pero no probé las otras versiones.
- No probé otros compiladores, habría que probar GCC y el AOCC de AMD
- Justo antes de subir esto al github probé con 36 hilos y el rendimiento fue de 2150 spins/ms en la versión 7_omp_sin_mejoras y 2050 spins/ms en la versión 6_omp. Habría que ponerse a probar de manera exhaustiva, pero tampoco quiero consumir todos los recursos del server boludeando con esto. Las veces anteriores que probé con tantos hilos el rendimiento era bajo, capaz porque el server estaba ocupado con otras cosas.

## Comandos útiles:
Uso esto para compilar:
```bash
icx -Wall -Wextra ising.c tiny_ising.c xoshiro256plus.c -fopenmp -lm -O2 -march=native -ipo -flto -fvectorize -funroll-loops -ffast-math -DL=4096
```

Uso esto para correr:
```bash
OMP_NUM_THREADS=24 ./a.out
```

Si quiero verificar que secciones/instrucciones son las que más consumen, uso esto:
```bash
OMP_NUM_THREADS=24 perf record ./a.out
perf report
```