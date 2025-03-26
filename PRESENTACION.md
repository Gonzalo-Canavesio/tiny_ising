
La métrica elegida para medir el desempeño es cantidad de spins sobre milisegundos. 

Esta medida es agnóstica del tamaño del problema, la cantidad de iteraciones que se realizan sobre cada elemento de la cuadricula (cada spin) es la misma en cualquier tamaño.

Los dispositivos utilizados para hacer las pruebas son:
- Cristibook: 
  - CPU: dual core Intel Celeron N4000 @ 1.10GHz
  - Memoria:
    - RAM: 4GiB DDR4 2400 MHz (2GiB x 2)
    - Cache L1: 112 KiB
    - Cache L2: 4 MiB
  - SO: Linux Mint 22 (Wilma)
  - Kernel: 6.8.0-53-generic x86_64
- PC Escritorio:
  - CPU: 12th Gen Intel(R) Core(TM) i5-12400F @ 2.5GHz
  - Memoria:
    - RAM: 32GiB DDR4 2400 MHz (16GiB x 2)
    - Cache L1: 80 KiB (por core)
    - Cache L2: 1.25 MiB (por core)
    - Cache L3: 18 MiB (compartida)
  - SO: Microsoft Windows 11 Pro (v10.0.22631)
- Server Atom:
  - CPU: AMD EPYC 7643 48-Core Processor @ 2.3 GHz
  - Memoria:
    - RAM: 128GiB DDR4
    - Cache L1: 64 KiB (por core)
    - Cache L2: 512 KiB (por core)
    - Cache L3: 256 MiB (compartida)
  - SO: Debian 13 (trixie)
  - Kernel: 6.12.12-amd64 x86_64

## Parte 1

El primer paso realizado fue optimizar el código. 

- En la primera versión se hace precomputo de unos exponenciales necesarios en el contexto del problema, lo que en la versión base representaba aproximadamente entre el 20% y el 30% del tiempo de computo del programa.
- En la segunda versión se cambia la función utilizada para calcular el random a xoshiro256+, ya que el calculo de randoms representaba aproximadamente entre el 20% y el 30% del tiempo de computo del programa. xoshiro256+ es más eficiente computacional y estadisticamente que rand() de la stdlib, que era lo que estabamos utilizando en la versión anterior.
- Se intentó modificar el tipo de los valores que se encargan de almacenar el valor de los spins (-1,1), actualmente son tipo int y la idea era migrarlos a int8_t, int_fast8_t o int_least8_t, pero el rendimiento decrecia en todos los casos.

Se realizaron pruebas con un tamaño L=256 y L=1024 (L es el largo del lado de la cuadrilla), usando el compilador GCC versión 14.2.0
Los resultados obtenidos son los siguientes: (Mostrar gráfico de barras y hablar un poco sobre los resultados)

## Parte 2

Una vez optimizado el código, pasamos a optimizar el compilador, para ello, vamos a probar distintos compiladores con las mejores flags de compilación que provea cada uno, priorizando siempre el time to solution (y que no se rompan los resultados con respecto a la versión no optimizada por el compilador). Esto requirió una busqueda de documentación, articulos y varias pruebas comparativas, llegando a la conclusión de que las mejores flags para el problema fueron:
- GCC versión 14.2.0: -O2
  - La flag -march=native empeoró los resultados en todos los dispositivos.
  - Las flags -O2, -O3, -Ofast daban un desempeño similar entre ellas en la mayoria de los casos, se eligió la -O2 porque en los casos de tamaño grande (L=1024) superaba de manera clara a las otras dos flags al realizar pruebas en la cristibook.
- Clang versión 19.1.7: 
- ICX (Intel) versión
- Compilador nvidia (nvcc?)
- Compilador AMD (aocc?)

(En el gráfico, agregar comparación contra la versión base)

## Parte 3 (capaz?)
Acá deberíamos ver si funciona mejor usando tipos int8 fast una vez aplicamos todas las optimizaciones y verificarlo con los 5 compiladores
Capaz -Ofast funciona bien ahora (???)
