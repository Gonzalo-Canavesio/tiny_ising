
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

El primer paso realizado fue optimizar el código. 

- En la primera versión precomputo los calculos de unos exponenciales que en la versión base representaba aproximadamente entre el 20% y el 30% del tiempo de computo del programa.
- En la segunda versión se cambia la función utilizada para calcular el random a xoshiro256+, ya que el calculo de randoms representaba aproximadamente entre el 20% y el 30% del tiempo de computo del programa. xoshiro256+ es más eficiente computacional y estadisticamente que rand() de la stdlib, que era lo que estabamos utilizando.
- En la tercera versión se actualizan los tipos de datos de los bucles para ser más eficientes en espacio, pasando de usar int (4 bytes) a int8_t 
