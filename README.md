# Tiny Ising - Computación Paralela

## Versiones

### Versión 1

Uso la versión dada por la catedra para comenzar las pruebas y experimentación, tomando medidas con distintos valores para L (largo de la cuadricula, que es de L*L casillas).

Luego de esta prueba voy a seleccionar cuales van a ser los valores de L que usaré para las mediciones y comparaciones.

### Versión 2

Luego de la versión 1 seleccione los mejores parametros para hacer las pruebas y comparaciones en las próximas etapas.

Acá reemplacé el uso de rand() de la stdlib por xoshiro256+, el cual uso para generar números entre 0.0 y 1.0 y obtener una probabilidad (aleatoria) que se utiliza en uno de los pasos del algoritmo.

### Versión 3

Vamos a comparar el rendimiento con distintas flags de compilación (O1,O2,O3,Ofast,Os) utilizando GCC, para verificar cual es la más rápida. 

Luego de varias pruebas, llegué a la conclusión que -Ofast es la flag que mejor rendimiento da en GCC.

Quise agregar la flag -march=native para habilitar todos los subconjuntos de instrucciones que son compatibles con la máquina local, pero en la netbook me dió un peor rendimiento usando esa flag.

### Versión 4

Ahora vamos a probar el rendimiento que brinda clang, con sus distintas optimizaciones.

Luego de hacer unas pruebas, clang superó a gcc y el mejor rendimiento fue dado por -Ofast, junto con la flag -march=native para habilitar todos los subconjuntos de instrucciones que son compatibles con la máquina local.

### Versión 5

Ahora vamos a probar el rendimiento que brinda el compilador de intel y cuales son las mejores flags para utilizar.

Luego de un par de pruebas, descubrí que este compilador me da mejor rendimiento que clang, en este caso utilizando la flag -fast.

### Versión 6

Vamos a intentar optimizar los tipos utilizados.

