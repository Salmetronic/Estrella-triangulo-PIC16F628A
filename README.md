## 🛡️ Propiedad Intelectual y Marca
© [2026] Salmetronic. Todos los derechos reservados. 
Los archivos de diseño de hardware en este repositorio se comparten con fines educativos y de código abierto, **pero el nombre comercial "Salmetronic" y el logotipo oficial están protegidos**. Queda totalmente prohibida su utilización para la comercialización o distribución de productos de terceros sin autorización previa.

<<<<<<< HEAD
# EstrellaTriangulo
=======
# Arrancador estrella-triángulo con PIC16F628A

Temporizador para el arranque de un motor trifásico en estrella-triángulo, programado en C con el compilador **XC8** para un **PIC16F628A** a 4 MHz.

## Funcionamiento

1. **Reposo:** los tres contactores están apagados.
2. **INICIO (RA1):** se activan el contactor de línea y el de estrella, y arranca el Timer1.
3. **Cambio a triángulo:** pasado el tiempo configurado se abre estrella, se espera un tiempo muerto de 50 ms y se cierra triángulo. La línea se mantiene.
4. **PARO (RB0):** apaga los tres contactores, detiene el temporizador y vuelve a reposo.

Medidas de seguridad incluidas:

- INICIO solo se acepta con el motor parado y el pulsador debe soltarse antes de volver a arrancar.
- Tiempo muerto entre estrella y triángulo para evitar que ambos contactores estén cerrados a la vez.
- PARO funciona por interrupción externa y, además, por sondeo en el bucle principal.
- Antirrebote por software en los pulsadores.

## Conexiones

| Pin | Función | Notas |
|---|---|---|
| RA1 | Pulsador INICIO | A GND, con pull-up externa de 10 kΩ a +5 V |
| RB0 | Pulsador PARO | A GND (pull-up interna activada; se recomienda pull-up externa en simulación) |
| RB1 | Contactor LÍNEA | Salida |
| RB2 | Contactor ESTRELLA | Salida |
| RB3 | Contactor TRIÁNGULO | Salida |

MCLR a +5 V (o `MCLRE` desactivado en los config bits).

## Configuración del tiempo

En `newmain.c`:

```c
#define TICKS_ESTRELLA      40      // Tiempo en estrella = 40 * 500 ms = 20 s
```

Tiempo en estrella = `TICKS_ESTRELLA` × 0,5 s.

El Timer1 trabaja con prescaler 1:8 a 4 MHz y recarga `0x0BDC`, lo que da 500 ms por desbordamiento.

## Compilación

- IDE: MPLAB X
- Compilador: XC8 v3.10
- Dispositivo: PIC16F628A
- Es necesario seleccionar el pack **PIC16Fxxx_DFP** en las propiedades del proyecto (*Packs*).
- Compilar en configuración *Production* para generar el `.hex`.

## Simulación

Probado en Proteus: cargar el `.hex` en el PIC, fijar el reloj a 4 MHz y accionar los pulsadores. Los contactores se pueden representar con LEDs.

## Autor

SALMETRONIC
>>>>>>> d4d8c15d233b1c7d5eeae168163c7ef8ed07bdf9
