/*
*Arxiu: funcions_micro.c
*Autor: Kevin Brandon Ventura Laura, Raúl Casado Luque
*Creat: 05/06/2026
*Descripció: funcions internes del microcontrolador, per gestionar el temps i la comunicació(I2C)
*/
/********************************************************************/ 
//Llibreries del microcontrolador i mòdul propi
#include <msp430.h>
#include <stdint.h>
#include "funcions_micro.h"
/********************************************************************/
//variable global de delay, per utilitzar-la a l'interrupció del timer B0
volatile uint16_t count_ms = 0;  // Comptador ms incrementat per l'ISR del Timer B0, és declarat com a "volatile" perquè canvia dins d'una interrupció.
/********************************************************************/
//variables globals per al protocol I2C
uint8_t *PTxData; // Punter (array) que conté les dades que volem enviar per I2C
uint8_t TXByteCtr; //Comptador de bytes que queden pendents de ser enviats pel bus
uint8_t *PRxData; //Punter on anirem desant els bytes rebuts per I2C
uint8_t RXByteCtr; // Comptador de bytes que queden pendents de rebre abans de tancar la connexió
/********************************************************************/
//Funció de inicialitzar timer B0,
void init_TimerB0(void) {
    // Configuraó del Timer B0: 
    // TBSSEL__SMCLK: Rellotge intern SMCLK de 16 MHz com a font de polsos
    // ID_0: Divisor de rellotge 1, per tant, el temporitzador de 16 MHz avança 16.000.000 de cops per segon
    // MC__UP: Mode UP, el timer B0 compta des de 0 fins al valor de TB0CCR0
    // TBCLR: Neteja i posa a zero el comptador
    TB0CTL = TBSSEL__SMCLK | ID_0 | MC__UP | TBCLR;
    TB0CCR0 = 16000; // Com que SMCLK va a 16MHz, per la qual cosa és 1ms (1msx(1s/1000ms)*16000000Hz = 16000, per tant, cada tick és 16000)
    TB0CCTL0 |= CCIE; //Habilitar la interrupció de comparació del registre CCR0 (s'activarà cada vegada que passi 1ms)
}
/********************************************************************/
//Funció de delay_ms
void delay_ms(uint16_t ms){
    count_ms = 0; // Reiniciar el comptador global de mil·lisegons a 0
    TB0CTL |= TBCLR; // Netejar el valor actual del comptador del Timer B1 per començar des de zero (valor conegut)
    TB0CCTL0 |= CCIE; // Activar interrupció del canal CCR0 
    TB0CTL |= MC__UP; //Mode Up, de recompte 
    // El bucle while funciona amb ISR i espera fins que la interrupció hagi incrementat count_ms tants cops com els "ms" demanats
    while(count_ms < ms){
        __no_operation(); // No fer res fins un cicle d'espera 
    }
    TB0CTL &= ~(MC__UP);  // Una vegada arribat al temps, s'atura el Timer apagant els bits del mode de recompte per estalviar energia
}

/********************************************************************/
//Inicialització del I2C (eUSCI_B1)
void i2c_init()
{
    P4SEL0 |= (BIT7 | BIT6); //BIT7 + BIT6; Configurar els pins P4.6 (SDA) i P4.7 (SCL) com I2C 

    UCB1CTLW0 |= UCSWRST; // Posar el mòdul eUSCI_B1 en estat de "Reset" per poder modificar la seva configuració de manera 
    
    //Configuració USB1CTLW0
    // UCMST: Configura el microcontrolador com a Mestre del bus
    // UCMODE_3: Mode de funcionament en mode I2C
    // UCSSEL_2: SMCLK (rellotge intenr de 16 MHz) com a rellotge base per generar la velocitat del bus
    UCB1CTLW0 |= UCMST | UCMODE_3 | UCSSEL_2;
    
    //Configurar la velocitat del rellotge I2C, SCL a standard mode 100 KHz:
    // Prescaler de 16 MHz/160=100KHz. 
    // Configura la velocitat del rellotge I2C (SCL) a Standard Mode (100 kHz):
    // Prescaler = 16 MHz / 160 = 100 kHz. El registre es divideix en byte baix BR0 i alt BR1
    UCB1BR0 = 160; // Byte baix del divisor 
    UCB1BR1 = 0; // Byte alt del divisor 
    UCB1CTLW0 &= ~UCSWRST;  // Allibera el mòdul eUSCI_B1 del reset per activar-lo amb la nova configuració

    //Configurar interrupcions del I2C (UCB1IE):
    // UCTXIE0: Interrupció de buffer de transmissió buit (llest per enviar)
    // UCRXIE0: Interrupció de buffer de recepció ple (dada rebuda)
    // UCNACKIE: Interrupció en cas que el dispositiu esclau enviï un No-Acknowledge (error / desconnexió)

    UCB1IE |= UCTXIE0 | UCRXIE0 | UCNACKIE;
}

/********************************************************************/
//Funció per enviar dades per I2C
void I2C_send(uint8_t addr, uint8_t *buffer, uint8_t n_dades)
{
    UCB1I2CSA = addr; // Assignar l'adreça del dispositiu esclau amb el qual ens comuniquem
    PTxData = buffer; // Apuntar el punter cap a l'adreça de memòria de les dades a enviar
    TXByteCtr = n_dades; // Desar el nombre total de bytes que s'han d'enviar per controlar el final del bucle
   
    //Configuració de UCB1CTLW0:
    // UCTR: Configurar el mòdul en mode Transmissor (Write)
    // UCTXSTT: Genera la condició de START a les línies del bus per començar la transmissió
    UCB1CTLW0 |= UCTR | UCTXSTT; 
    // Atura la CPU principal entrant en Mode de Baix Consum 0 (LPM0) amb les interrupcions globals actives (GIE). 
    // La CPU "s'adorm" mentre el maquinari de l'I2C fa la feina byte a byte mitjançant les interrupcions.
    __bis_SR_register(LPM0_bits + GIE); //entrar a baix consum amb les interrupcions globals actives, és a dir, s'adorm mentre el I2C envia byte a byte mitjançant el ISR
    __no_operation(); // No fer res i esperar de nou la interrupció
    while (UCB1CTLW0 & UCTXSTP); // Esperar bloquejant fins que el bit de la condició de STOP (UCTXSTP) s'hagi netejat del bus
}
/********************************************************************/
//Funció de recepció de dades
void I2C_receive(uint8_t addr, uint8_t *buffer, uint8_t n_dades)
{
    PRxData  = buffer; // Apuntar el punter cap a l'adreça de memòria on s'ha desat de les dades a enviar
    RXByteCtr = n_dades; // Estableix el nombre de bytes que esperem rebre del dispositiu esclau
    UCB1I2CSA  = addr; // Configura l'adreça de l'esclau al qual demanarem les dades          
    UCB1CTLW0 &= ~UCTR; // Netejar el bit UCTR per configurar el mòdul en mode Receptor (Read)          
    while (UCB1CTLW0 & UCTXSTP);  // Assegurar que el bus no té cap condició de STOP pendent de transmissions anteriors
    UCB1CTLW0 |= UCTXSTT;  // Generar condició de START al bus per demanar les dades a l'esclau        
    __bis_SR_register(LPM0_bits + GIE);//entrar a baix consum amb les interrupcions globals actives, és a dir, s'adorm mentre el I2C envia byte a byte mitjançant el ISR
    __no_operation();// No fer res i esperar de nou la interrupció
}

/********************************************************************/
//INTERRUPCIONS
/********************************************************************/
//interrupció timerB0 (increment del comptador definit)
#pragma vector = TIMER0_B0_VECTOR //Vector d'interrupció del timer B0
__interrupt void Timer_B0_ISR (void) //Inici d'interrupció
{
    count_ms++;   // Incrementa en 1 la variable global de mil·lisegons cada vegada que el Timer arriba a un tick (16000 per com es va configurar el timer B0)
}

/********************************************************************/
//Interrupcions per l'I2C (eUSCI_B1) 
#pragma vector = USCI_B1_VECTOR  //Vector d'interrupció de comunicacions USCI_B1
__interrupt void ISR_USCI_I2C(void) //Inici d'interrupció de l'I2C
{
    //Amb __even_in_range s'optimitza el codi de salt avaluant el registre  d'interrupcions fins a un màxim de 0x1E
    switch(__even_in_range(UCB1IV, 0x1E))
    {
        case USCI_NONE: break; // Cas 0: No hi ha cap interrupció pendent, surt directament

        case USCI_I2C_UCNACKIFG: // Cas NACK: L'esclau no ha respost o ha rebutjat la comunicació
            UCB1CTLW0 |= UCTXSTP; // Generar condició de STOP d'emergència per alliberar el bus
            UCB1IFG &= ~UCNACKIFG; // Netejar la flag d'interrupció de NACK
            __bic_SR_register_on_exit(LPM0_bits); // Desperta la CPU al sortir de la interrupció per informar de l'error
            break;

        case USCI_I2C_UCRXIFG0:// Cas RECEPCIÓ: Ha arribat un nou byte de l'esclau al buffer d'entrada
            if (RXByteCtr) { // Si encara queden bytes per llegir segons el nostre comptador
                *PRxData++ = UCB1RXBUF; // Llegeix el byte del registre UCB1RXBUF, desar i avança el punter
                if (RXByteCtr == 1) UCB1CTLW0 |= UCTXSTP; // Si aquest que acabem de rebre era el penúltim, activem el STOP per al següent
            } else { // Si el comptador ha arribat a 0 (últim byte)
                *PRxData = UCB1RXBUF;  // Desar el darrer byte rebut
                __bic_SR_register_on_exit(LPM0_bits);// Desperta la CPU del mode LPM0 perquè la lectura ha finalitzat amb èxit
            }
            RXByteCtr--; // Decrementa el comptador de bytes pendents de recepció
            break;

        case USCI_I2C_UCTXIFG0:  // Cas TRANSMISSIÓ: El buffer d'I2C està lliure i preparat per enviar el següent byte
            if (TXByteCtr) { // Si el comptador diu que encara queden bytes per transmetre
                UCB1TXBUF = *PTxData++; // Carrega la dada apuntada pel punter al registre de transmissió (això és automàtic)
                TXByteCtr--; // Decrementa el comptador de bytes que queden a l'array
            } else {
                UCB1CTLW0 |= UCTXSTP; // Generar condició de STOP per finalitzar la comunicació al bus de forma correcta
                UCB1IFG &= ~UCTXIFG0;// Netejar flag d'interrupció de transmissió
                __bic_SR_register_on_exit(LPM0_bits); // Desperta la CPU del mode LPM0 per continuar amb la següent línia de codi del main
            }
            break;

        default: break;  // Ignora qualsevol altre cas no contemplat (com ara flags de START, etc.)
    }
}

