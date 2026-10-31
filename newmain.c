/*
 * File:   newmain.c
 * Author: SALMETRONIC
 * Circuito arranque de un temporizador estrella-triangulo
 * Created on 6 de noviembre de 2023, 18:56
 */

#include <xc.h>
#include "config.h"

#define LINEA RB1   //Contactor principal
#define ESTRELLA RB2    //Contactor Estrella
#define TRIANGULO RB3   //Contactor Triangulo
#define INICIO RA1      //Pulsador de inicio
#define PARO RB0        //Pulsador de paro.

int contador = 1;   //Variable de control.
int i=0;    //Variable i condiciona el funcionamiento del pulsador de inicio.

//Interrupciones por entrada externa y por TIMER1

void __interrupt() isr(){
    if (INTCONbits.INTF){
        INTCONbits.INTF=0;  //Limpia bandera
        LINEA=0;    //Contactor de línea apagado.
        ESTRELLA=0;     //Contactor de ESTRELLA apagado.
        TRIANGULO=0;    //Contactor de TRIÁNGULO apagado.
        i=0;    //Apaga el circuito, motor apagado.
        TMR1H=0x00;   //Cargar en el registro alto del temporizador 500ms. 
        TMR1L=0x00;   //Cargar en el regitro bajo del temporizador 500ms.
        T1CONbits.TMR1ON=0; //Detiene el timer.
        contador=0; //Aqui estaba el error.
         
        }
   if (PIR1bits.TMR1IF&&PIE1bits.TMR1IE){
       PIR1bits.TMR1IF=0;
       contador++;
       if(contador==40&&i==1){  //Cambiando en numero del contador cambias el tiempo de arranque.
               //Si la bandera de inicio esta activa.
         i=0;    //Desactiva el pulsador de inicio.El motor está funcionando 
         ESTRELLA=0;    //Apaga contactor ESTRELLA.
         TRIANGULO=1;   //Enciende contactor TRIANGULO.
         contador=0;
       }
       
        }
    }
    

// Configuración de TIMER1.
void TIMER1_Config(){
    OPTION_REG= 0x02; //prescsler 8
    TMR0=131;   //1ms
    //T1CON= 0b00110101; //prescasler 8 T1CON= 0b00110101
    //TMR1H=0x00;   //Reinicio contador.
    //TMR1L=0x00;   //Reinicio contador.
    PIE1bits.TMR1IE=1;  //Habilita la interrupción.
    PIR1bits.TMR1IF=0;  //Limpia la bandera.
    INTCONbits.GIE=1;  //Habilita las interrupciones globales.
    INTCONbits.PEIE=1;  //Habilita las interrupciones perifericas.
    INTCONbits.INTE=1;  //Habilia la interrupción externa.
    OPTION_REGbits.INTEDG=0;    //Configurar la interrupción por flanco de bajada.
}

void main(void) {
    CMCON=7;  //DESHABILITA COMPARADORES ANALOGICOS
    // Configuración de puertos
    TRISB = 0x01;  // RB0 como entrada, el resto como salidas
    TRISA = 0x02;  // RA1 como entrada, el resto como salidas
    PORTB = 0x00;
    TIMER1_Config();    //Configuración presscaler 8
 
    
       
    while (1) {       
        
        if (INICIO == 0&&ESTRELLA==0) { //Arranca el motor si cumple condiciones.
            TMR1H=0xB;   //Cargar en el valor alto 500ms 0x11
            TMR1L=0xDC;   //Cargar en el valor alto 500ms 0x20
            T1CONbits.TMR1ON=1; //Inicia el timer.
             
                     
            i=1; //Condición para el arranque del motor.
            LINEA=1;    //Enciende contactor LINEA.
            ESTRELLA=1; //Enciende contactor ESTRELLA.
        
            }   
        
        
    }
return;
    }



