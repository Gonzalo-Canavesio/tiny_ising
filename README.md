# Tiny Ising - Computación Paralela

## Versiones

### Versión 1.0.0

- **Largo de lado**: 384

### Versión 1.0.1

- **Largo de lado**: Siempre tiene que ser potencia de 2

#### Observaciones

Si hacemos que el "largo de lado" sea potencia de 2, mejora bastante el rendimiento. 

### Versión 2.0.0

#### Observaciones

Intenté hacer que se use 1 solo bit por cada spin (es decir, 1 byte tenia 8 spines), salió todo mal, el rendimiento bajó a la mitad. Probablemente se debió al overhead que agregaba el tener que calcular la posición y obtener el bit correcto.
