/*
*Arxiu: uart_alumnos.c
*Autor: Kevin Brandon Ventura Laura, Raúl Casado Luque
*Creat: 19/06/2026
*Descripció:Comunicació del mòdul WIFI ESP-10S amb els components del robot
*/
/********************************************************************/ 
//Llibreries del microcontrolador i mòdul del professor
#include <msp430.h>
#include <stdint.h>
#include "uart_alumnos.h"
/********************************************************************/
// Variables de control que ens serviràn al ISR 
static volatile uint8_t  DatoLeido_UART = 0; // Desar l'últim byte llegit des del registre de recepció UART
static volatile uint8_t  Byte_Recibido  = 0;  // Flag indicador (0 = No ha arribat res, 1 = Byte rebut a la ISR)
static volatile uint32_t timeout_counter = 0;// Comptador incremental de temps gestionat per la interrupció del Timer B2

/********************************************************************/
//Funció per configurar el timer B2 (base 100us)
static void Activa_Timer_Timeout(void) {
    //Configuració del timer B2:
    // TBSSEL__SMCLK: Rellotge de intern 16MHz com a font de polsos
    // ID_0: Divisor de rellotge entre 1, directament és 16MHz
    // MC__UP: Mode de funcionament up, compta des de 0 fins al valor de TB2CCR0
    // TBCLR: Netejar i posar a zero el registre del comptador 
    TB2CTL   = TBSSEL__SMCLK | ID_0 | MC__UP | TBCLR;
    TB2CCR0  = 1600 - 1;   //(100us*1/1000000*16000000 = 1600, 1600tick per cada 100us )
    TB2CCTL0 = CCIE; //Habilitar la interrupció per comparació del canal 0 del Timer B2
    timeout_counter = 0;// Inicialitzar el comptador de cicles a 0, abans de començar la mesura
}


/********************************************************************/
//Funció per reiniciar el comptador del timeout a zero per tornar a començar a mesurar des d'aquest instant
static void Reset_Timeout(void) {
    timeout_counter = 0;
}
/********************************************************************/
//Funció que comprova si el temps transcorregut ha assolit o superat time_out (en unitats de 100us)
static uint8_t TimeOut(uint32_t time_out) {
    return (timeout_counter >= time_out) ? 1 : 0;  // Retorna 1 si s'ha esgotat el temps d'espera, 0 si encara queda temps
}
/********************************************************************/
//Funció que atura el temporitzador del timeout i bloqueja les seves interrupcions per seguretat i estalvi d'energia
static void Desactiva_Timer_Timeout(void) {
    TB2CTL   &= ~MC__UP; // Apagar els bits del mode up
    TB2CCTL0 &= ~CCIE; // Desactivar la interrupció de comparació del canal de control del Timer
}

/********************************************************************/
//Inicialització de mòdul wifi
void init_uart_wifi(void)
{
    // ENABLE del ESP-01: P4.4 a HIGH
    P4DIR |=  BIT4; // Configurar com a sortida
    P4OUT |=  BIT4; // Posar a HIGH per activar el mòdul Wi-Fi
    // Configuració de les línies de comunicació sèrie ( RXD i TXD)
    P4SEL0 |=  (BIT2 | BIT3);  // Configurar els pins P4.2 (RX) i P4.3 (TX) com a funció de comunicació UART
    P4SEL1 &= ~(BIT2 | BIT3); // Netejar els bits de funció secundària per assegurar l'enrutament de la UART
    UCA1CTLW0 |= UCSWRST; // Posar la línia UART UCA1 en estat de reset USCI per configurar-la amb seguretat (inoperativa)       
    UCA1CTLW0 |= UCSSEL__SMCLK;    // SMCLK 16MHz como fuente BRCLK 
    //BAUD RATE A 115200 BPS
    UCA1MCTLW  = UCOS16;           // Activar el mode de sobre-mostreig (Oversampling=1) per reduir errors de rellotge a velocitats altes
    UCA1BRW    = 8;                 // prescaler BRCLK         
    UCA1MCTLW |= (10   << 4);      // Assigna el valor de modulació de primer estadi UCBRFx = 10 als bits[7:4] del registre       
    UCA1MCTLW |= (0xF7 << 8);       // Assigna el valor de modulació de segon estadi UCBRSx = 0xF7 als bits[15:8] del registre      
    UCA1CTLW0 &= ~UCSWRST;         // Reactivar UART              
    UCA1IE  |=  UCRXIE;// Habilitar la interrupció que s'activa en rebre un byte complet (RX)
    UCA1IFG &= ~UCRXIFG; // Netejar qualsevol flag residual de recepció per començar en net
}

/********************************************************************/
//Funció de transmissió de paquet de dades
uint8_t TxPacket(uint8_t bParameterLength, const uint8_t *Parameters)
{
    uint8_t i;  // Variable control per recórrer el bloc d'enviament
    for (i = 0; i < bParameterLength; i++) { // Bucle per cadascun dels bytes demanats per enviar
        while (!(UCA1IFG & UCTXIFG));  // Esperar TX ready  
        UCA1TXBUF = Parameters[i]; // Posa el byte de dades actual al registre d'enviament ( inicia la transmissió)
    }
    // Verificar si el buffer està buit abans de sortir
    while (UCA1STATW & UCBUSY);// Espera bloquejant mentre el mòdul UART estigui transmetent activament
    return bParameterLength;// Retorna la quantitat total de bytes que s'han enviat correctament
}

/********************************************************************/
//Funció de rebre paquet de dades
RxReturn RxPacket(uint32_t time_out)
{
    RxReturn respuesta; // Estructura on es desa la col·lecció de dades rebudes i el seu estat
    uint16_t bCount;  // Índex de posició i de recompte de bytes emmagatzemats al buffer
    uint8_t  Rx_time_out = 0; // Flag que s'activarà en cas de patir una fallada de temps (timeout)

    respuesta.num_bytes = 0;  // Inicialitza a zero el recompte de bytes capturats de la resposta
    respuesta.time_out  = 0; // Inicialitza a zero l'estat d'error de timeout de l'estructura

    Activa_Timer_Timeout(); // Engega i configura el temporitzador de 100µs per controlar el flux de temps de recepció

    for (bCount = 0; bCount < RX_BUFFER_SIZE; bCount++) // Bucle per recollir dades 
    {
        Reset_Timeout();  // Posar el timeout a zero abans de començar l'espera de cada byte nou
        Byte_Recibido = 0;  //Posar el flag a zero per indicar que encara s'està esperant l'arribada de dades

        while (!Byte_Recibido) // Bucle d'espera bloquejant mentre la interrupció sèrie (ISR) no marqui que ha entrat un byte
        {
            Rx_time_out = TimeOut(time_out);        // Avalua si s'ha superat el temps límit indicat
            if (Rx_time_out) break;                 // Si el temps d'espera s'ha esgotat, surt del bucle while
        }

        if (Rx_time_out) break;                      // Si hem sortit de l'anterior bucle en esgotar-se timeout, trenca també el bucle global for

        // Si el byte s'ha rebut correctament a temps
        respuesta.StatusPacket[bCount] = DatoLeido_UART; // Copia la dada emmagatzemada per la ISR a la matriu de dades de sortida
    }

    respuesta.num_bytes = bCount; // Enregistra a l'estructura el nombre final de bytes llegits abans d'aturar-se
    respuesta.time_out  = Rx_time_out;  // Desar l'estat de fallada de temps

    Desactiva_Timer_Timeout(); // Apagar el temporitzador de control de temps per estalviar recursos
    return respuesta; // Retorna l'estructura completa amb el resultat definitiu de la recepció
}

/********************************************************************/
//INTERRUPCIONS
/********************************************************************/
//Interrupcions de recepció serie UART UCA1
#pragma vector = USCI_A1_VECTOR //vector interrupció del USCI_A1
__interrupt void EUSCIA1_IRQHandler(void) //inici de interrupcions
{
    if (UCA1IFG & UCRXIFG) {
        UCA1IE         &= ~UCRXIE;      // Desactivar interrupcions en RX 
        DatoLeido_UART  = UCA1RXBUF;    // Copia el byte rebut del registre, això neteja el flag RX
        Byte_Recibido   = 1;           // Activar el flag global per notificar a la funció bloquejant RxPacket que la dada és vàlida
        UCA1IE         |=  UCRXIE;      // Activar de nou les interrupciones en RX
    }
}

/********************************************************************/
//interrupció timerB2 temporitzador timeout  
#pragma vector = TIMER2_B0_VECTOR  //Vector d'interrupció del timer B2
__interrupt void Timer_B2_ISR(void)//Inici d'interrupció
{
    timeout_counter++; // Incrementa en 1 el comptador cada vegada que expira el termini de temps 100us
}

