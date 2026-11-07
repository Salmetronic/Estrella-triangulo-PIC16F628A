/*
 * File:   newmain.c
 * Author: SALMETRONIC
 * Arrancador estrella-triangulo con temporizador (PIC16F628A, XC8)
 *
 * Funcionamiento:
 *  1. Reposo: los tres contactores apagados.
 *  2. INICIO (RA1, nivel bajo): se activan LINEA + ESTRELLA y arranca el Timer1.
 *  3. Pasado el tiempo configurado se abre ESTRELLA, se espera un tiempo muerto
 *     y se cierra TRIANGULO. LINEA se mantiene.
 *  4. PARO (RB0, flanco de bajada, interrupcion externa): apaga todo y reinicia.
 *
 * Base de tiempos (Fosc = 4 MHz, Timer1 con prescaler 1:8):
 *  Tick = 8 us. Recarga 0x0BDC (3036) -> 62500 ticks = 500 ms por desbordamiento.
 *  Tiempo en estrella = TICKS_ESTRELLA * 500 ms (40 -> 20 s).
 *
 * Created on 6 de noviembre de 2023, 18:56
 */

#include <xc.h>
#include "config.h"

#ifndef _XTAL_FREQ
#define _XTAL_FREQ 4000000UL        // Necesario para __delay_ms()
#endif

/* ----------------------------- Pines ----------------------------------- */
#define LINEA       RB1             // Contactor principal
#define ESTRELLA    RB2             // Contactor estrella
#define TRIANGULO   RB3             // Contactor triangulo
#define INICIO      RA1             // Pulsador de inicio (activo en 0)
#define PARO        RB0             // Pulsador de paro   (activo en 0, INT)

/* --------------------------- Parametros -------------------------------- */
#define TMR1_RECARGA_H      0x0B    // 3036 = 0x0BDC -> 500 ms
#define TMR1_RECARGA_L      0xDC
#define TICKS_ESTRELLA      10      // Tiempo en estrella = 40 * 500 ms = 20 s
#define TIEMPO_MUERTO_MS    50      // Pausa entre abrir ESTRELLA y cerrar TRIANGULO
#define ANTIREBOTE_MS       20

/* ------------------------------ Estados -------------------------------- */
#define ST_PARADO       0
#define ST_ESTRELLA     1
#define ST_TRIANGULO    2

/* --------------------------- Variables globales ------------------------ */
volatile unsigned char estado = ST_PARADO;      // Estado del arrancador
volatile unsigned char contador = 0;            // Desbordamientos de Timer1
volatile unsigned char fin_estrella = 0;        // Aviso ISR -> main: hacer la transicion

/* ----------------------------- Prototipos ------------------------------ */
void Config_Puertos(void);
void Config_Timer1(void);
void Config_Interrupciones(void);
void Parar_Todo(void);
void Arrancar(void);
void Pasar_A_Triangulo(void);

/* ------------------------------ Interrupciones ------------------------- */
void __interrupt() isr(void)
{
    // PARO: interrupcion externa RB0
    if (INTCONbits.INTF) {
        INTCONbits.INTF = 0;
        Parar_Todo();
    }

    // Timer1: base de tiempo de 500 ms
    if (PIR1bits.TMR1IF && PIE1bits.TMR1IE) {
        PIR1bits.TMR1IF = 0;
        TMR1H = TMR1_RECARGA_H;             // Recarga para mantener los 500 ms
        TMR1L = TMR1_RECARGA_L;

        if (estado == ST_ESTRELLA) {
            contador++;
            if (contador >= TICKS_ESTRELLA) {
                fin_estrella = 1;           // La transicion la hace main()
            }
        }
    }
}

/* ------------------------------ Funciones ------------------------------ */
void Config_Puertos(void)
{
    CMCON = 0x07;                   // Comparadores analogicos desactivados
    TRISA = 0x02;                   // RA1 entrada, resto salidas
    TRISB = 0x01;                   // RB0 entrada, resto salidas
    PORTA = 0x00;
    PORTB = 0x00;                   // Contactores apagados
}

void Config_Timer1(void)
{
    T1CON = 0b00110000;             // Reloj interno, prescaler 1:8, Timer1 apagado
    TMR1H = TMR1_RECARGA_H;
    TMR1L = TMR1_RECARGA_L;
    PIR1bits.TMR1IF = 0;
    PIE1bits.TMR1IE = 1;            // Interrupcion de Timer1
}

void Config_Interrupciones(void)
{
    OPTION_REGbits.nRBPU = 0;       // Pull-up interna en PORTB (RB0)
    OPTION_REGbits.INTEDG = 0;      // INT0 por flanco de bajada
    INTCONbits.INTF = 0;
    INTCONbits.INTE = 1;            // Interrupcion externa RB0 (PARO)
    INTCONbits.PEIE = 1;            // Interrupciones de perifericos
    INTCONbits.GIE = 1;             // Interrupciones globales
}

// Apaga contactores, detiene el timer y vuelve a reposo.
void Parar_Todo(void)
{
    T1CONbits.TMR1ON = 0;
    LINEA = 0;
    ESTRELLA = 0;
    TRIANGULO = 0;
    TMR1H = TMR1_RECARGA_H;
    TMR1L = TMR1_RECARGA_L;
    contador = 0;
    fin_estrella = 0;
    estado = ST_PARADO;
}

// Arranque en estrella.
void Arrancar(void)
{
    contador = 0;
    fin_estrella = 0;
    TMR1H = TMR1_RECARGA_H;
    TMR1L = TMR1_RECARGA_L;
    PIR1bits.TMR1IF = 0;

    TRIANGULO = 0;                  // Seguridad: triangulo siempre apagado aqui
    LINEA = 1;
    ESTRELLA = 1;
    estado = ST_ESTRELLA;
    T1CONbits.TMR1ON = 1;
}

// Estrella -> triangulo con tiempo muerto para evitar cortocircuito entre contactores.
void Pasar_A_Triangulo(void)
{
    fin_estrella = 0;
    ESTRELLA = 0;                   // Primero se abre estrella
    __delay_ms(TIEMPO_MUERTO_MS);   // Tiempo muerto

    if (estado == ST_ESTRELLA) {    // Solo si no se pulso PARO durante la pausa
        TRIANGULO = 1;
        estado = ST_TRIANGULO;
        T1CONbits.TMR1ON = 0;       // El timer ya no hace falta
    }
}

/* -------------------------------- Main --------------------------------- */
void main(void)
{
    unsigned char inicio_liberado = 0;  // Evita rearranques por pulsador mantenido

    Config_Puertos();
    Config_Timer1();
    Config_Interrupciones();

    while (1) {

        // PARO por sondeo (respaldo de la interrupcion externa).
        // Si RB0 esta a 0 con el motor en marcha, se fuerza la misma rutina
        // de la ISR activando la bandera INTF por software.
        if (PARO == 0 && estado != ST_PARADO) {
            __delay_ms(ANTIREBOTE_MS);
            if (PARO == 0 && estado != ST_PARADO) {
                INTCONbits.INTF = 1;    // Dispara la ISR de paro
            }
        }

        // Transicion pedida por la ISR
        if (fin_estrella) {
            Pasar_A_Triangulo();
        }

        // Solo se admite INICIO con el motor parado
        if (estado == ST_PARADO) {
            if (INICIO == 1) {
                inicio_liberado = 1;            // Pulsador suelto: rearme
            }
            else if (inicio_liberado) {
                __delay_ms(ANTIREBOTE_MS);
                if (INICIO == 0 && estado == ST_PARADO) {
                    inicio_liberado = 0;
                    Arrancar();
                }
            }
        }
    }
}