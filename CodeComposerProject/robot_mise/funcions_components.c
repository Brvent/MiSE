/*
*Arxiu: funcions_components.c
*Autors: Kevin Brandon Ventura Laura, Raúl Casado Luque
*Creat: 05/06/2026
*Descripció: Control dels components integrats a la PCB de control de perifèrics del robot (joystick, ultrasons, LDRs i LED), gestiona la configuració de inicialització
*dels mòduls, funcionament i rutines d'iterrupció(ISR) relacionades
*/
/********************************************************************/ 
//Llibreries del microcontrolador i mòduls propis
#include <msp430.h> 
#include <stdint.h>
#include "funcions_micro.h"
#include "funcions_components.h"
/********************************************************************/
//variables joystick
volatile uint8_t estado_joystick = 0; //Variable que emmagatzema la direcció del joystick
volatile uint8_t modo_robot = 0; // Variable que emmagatzema el mode de funcionament del robot (0 a 4)
volatile uint16_t ultim_canvi_modo_tb1 = 0xFFFF; // Marcar de temps del registre TB1R del darrer canvi de mode, el valor inicial 0xFFFF perque la primera pulsacio sempre s'accepti
#define up      BIT6  //Estat de direcció del joystick, amunt
#define down    BIT7  //Estat de direcció del joystick, avall
#define left    BIT2  //Estat de direcció del joystick, esquerra
#define right   BIT3  //Estat de direcció del joystick, dreta
#define sel   BIT5    //Estat de joystick, selecció (modo)
/********************************************************************/ 
//variables ultrasons
volatile uint16_t captura_inicio = 0; // Variable per desar el valor del comptador quan el pols ECHO està en High
volatile uint16_t captura_fin = 0;   // Variable per desar el valor del comptador quan el pols ECHO està en Low
volatile uint8_t  captura_completada = 0; //Variable que serveix com flag per indicar si la mesura s'ha completat per un pols sencer (sí=1 i No=0)
volatile float   distancia_cm = 0.0; // Variable per emagatzemar el valor de la distància en cm.
/********************************************************************/ 
//Inicialització del led controlat per un PWM del micro 
void init_PWM_LED(void) {
    // configuració del pin P2.0 per fer la seva funció principal (Timer TB1.1) P2SELx (01) i P2DIR (1)
    P2SEL0 |= BIT0; //Posar en HIGH
    P2SEL1 &= ~BIT0; //Posar en LOW
    //La combinació de ambdues selecciona la funció principal
    P2DIR  |= BIT0; // P2.0 com a sortida, ja que s'envia el senyal PWM al LED
    
    //Configuració del timer:
    // TBSSEL__SMCLK: rellotge intern de 16 MHz
    // ID__8: Divisió de la freqüència del rellotge 8, (16/8) MHz= 2MHz
    // MC__CONTINUOUS: mode de funcionament continu, el timer compta des de 0x0000 fins 0xFFFF de forma cíclica
    // TBCLR: Neteja i posa a zero el comptador del Timer B1 (reset)
    // Nota: Aquesta configuració de timer ha de ser compatible amb el ultrasons, perquè com s'utilitza el mateix timer pot haver-hi un problema lectura del ultrasons.
    TB1CTL = TBSSEL__SMCLK | ID__8 | MC__CONTINUOUS | TBCLR;

    TB1CCR0 = 0; //Registre comparació, el pin posa HIGH quan iguala TB1CCR0 i posa a LOW quan iguala TB1CCR1, com s'ha posat a 0, el pols comença en alt.
    
    TB1CCTL1 = OUTMOD_7; //Configurar com sortida en mode de funcionament (Reset/set) per generar el duty cycle
    //Set posa en HIGH, el pin s'encén quan el temporitzador iguala el valor de TB1CCR0
    //Reset posa en LOW, el pin s'apaga quan el tempotizador iguala el valor de TB1CCR1

    TB1CCR1 = 0; // Inicialment apagat, duty cycle 0
    
    PM5CTL0 &= ~LOCKLPM5; // Desactiva el mode de baix consum i bloqueig d'I/O digital activant els pins
}

/********************************************************************/
//funció percentatge de brillantor del led
void LED_Percentatge(uint8_t percentage) {
    if (percentage > 100) { // Limit de brillantor
        percentage = 100; // Es força a un valor màxim del 100%
    }
    uint32_t calculado = ((uint32_t)percentage * PWM_PERIOD) / 100;  // Calcular el valor del registre de comparació a partir de PWM_PERIOD (definit a la capçalera)
    TB1CCR1 = (uint16_t)calculado; // Aplicar el valor obtingut al registre per canviar la brillantor del LED
}
/********************************************************************/
void LED_PWM(void) {
    static uint8_t brillo = 0; // Variable estàtica (memòria permanent)que recorda el nivell actual de brillantor entre crides (0 a 100)
    static int8_t direccio = 1;  // Variable estàtica, indica si la brillantor puja (1) o baixa (-1)

    LED_Percentatge(brillo); // Actualitzar el valor de la brillantor del LED

    brillo += direccio; // Variable que serveix per incrementar/decrementar el valor de brillantor en cada pas

    if (brillo >= 100) {  // Si s'arriba al límit superior de brillantor
        direccio = -1; // Es canvia el sentit cap avall (comença a apagar-se)
    } else if (brillo <= 0) { // Si s'arriba al límit inferior (apagat total)
        direccio = 1; // Es canvia el sentit cap amunt (comença a encendre's)
    }

    delay_ms(15);  // delay de 15ms per aconseguir observar el parpelleig
}

/********************************************************************/
// Inicializacio del ADC para los LDRs
void init_ldr(void){
    P5DIR &= ~(BIT0 | BIT1); // Configurar els pins P5.0 i P5.1 com a entrades         
    P5SEL0 |= (BIT0 | BIT1); // Seleccionar com entrada analògica A8 posant pins en HIGH (11) 
    P5SEL1 |= (BIT0 | BIT1); // Seleccionar com entrada analògica A9 posant pin en HIGH (11) 
    //Activant els canals analògics (ADC)
    //Configuració de ADCCTL0
     // ADCON: Encén el mòdul ADC per fer mesures del valor dels LDRs
    // ADCSHT_2: Defineix el temps de Sample and Hold a 16 cicles de rellotge de l'ADC
    ADCCTL0 |= (ADCON | ADCSHT_2);      
    ADCCTL1 |= (ADCSHP); // Defineix que l'impuls de mostra prové del temporitzador de mostreig intern del microcontrolador
    ADCCTL2 &= ~ADCRES; //Neteja els bits i defineix la resolució resultant de la conversió                 
    ADCCTL2 |= ADCRES_1; //Resolució del mòdul ADC a 10 bits (valors de 0 a 1023)
    ADCMCTL0 |= ADCSREF_0; // Defineix les referències de voltatge: V(R+) = AVCC i V(R-) = AVSS a l'alimentació del microcontrolador              

    ADCIE |= ADCIE0;  // Habilita la interrupció que s'activa quan una conversió ADC finalitza correctament                   
}

/********************************************************************/
// leer valor del canal A8 y A9
static uint16_t leer_ADC(uint8_t canal) {
    ADCCTL0 &= ~ADCENC;    // Desactiva la conversió ADC per poder modificar els paràmetres     
    ADCMCTL0 &= ~ADCINCH_15; // Neteja la selecció del canal d'entrada actual  
    ADCMCTL0 |= canal;     // Assignar el nou canal que es vol llegir (passat com a paràmetre)    

    ADCIFG &= ~ADCIFG0;  // No flag pendent d'interrupció del canal 0 per evitar falsos positius       
    ADCCTL0 |= ADCENC | ADCSC;  // ADCENC: Habilita el ADC. ADCSC: Inicia la conversió.
    __bis_SR_register(LPM0_bits | GIE); //Mode de baix consum 0 (LPM0) amb interrupcions actives esperant el resultat del canal.

    return ADCMEM0;   // Retorna el valor digitalitzat que s'ha guardat al registre de memòria de l'ADC (0 a 1023)          
}

/********************************************************************/
// valors dels LDRs 
uint16_t get_ldr_esq(void) {
    return leer_ADC(ADCINCH_8); // Crida a la funció de l'ADC passant el canal A8 (associat al LDR esquerre a P5.0)
}

uint16_t get_ldr_dre(void) {
    return leer_ADC(ADCINCH_9); // Crida a la funció de l'ADC passant el canal A9 (associat al LDR dret a P5.1)
}

/********************************************************************/
//Estat del joystick
uint8_t get_estado_joystick(void) {
     return estado_joystick; //Consultar l'estat actual de moviment (up,down,left,right)
    }
uint8_t get_modo_robot(void){
     return modo_robot; //Consultar el mode actual de funcionament
    }
void reset_estado_joystick(void) { 
    estado_joystick = 0; // Força l'estat del joystick a 0, una posició coneguda
 }
//Inicialitzar joystick
void init_entradas(void) {
    P3DIR &= ~(up | down | left | right | sel); // Configura tots els pins implicats del Joystick com a entrades
    P3REN |= sel; // Resistència interna (Pull-up/Pull-down) pel pin de selecció (sel)
    P3OUT |= sel;  // Resistència interna del pin "sel" com a Pull-up 
    P3IES |= (up | down | left | right | sel); // Configura les interrupcions de tots els pins per activar-se amb flanc de baixada (High a Low)
    P3IFG &= ~(up | down | left | right | sel); // Neteja totes les flags d'interrupció acumulades del Port 3 abans de començar
    P3IE  |= (up | down | left | right | sel);  // Habilita les interrupcions per a cadascun d'aquests pins
}

/********************************************************************/
//inicialitzar ultrasons
void init_ultrasons(void) {
    P2DIR |=  BIT3; // Configurar com sortida, trig del sensor
    P2OUT &= ~BIT3;  // El pin trig comença en LOW

    P2DIR  &= ~BIT1; // Configurar com a entrada, echo.
    P2SEL0 |=  BIT1; // Captura externa de temporitzador del pin P2.1 (TB1.CCI2A), Posar en HIGH
    P2SEL1 &= ~BIT1; //Posar en LOW
     //La combinació de ambdues selecciona la funció principal

    //Configuració del timer:
    // TBSSEL__SMCLK: rellotge intern de 16 MHz
    // ID__8: Divisió de la freqüència del rellotge 8, (16/8) MHz= 2MHz
    // MC__CONTINUOUS: mode de funcionament continu, el timer compta des de 0x0000 fins 0xFFFF de forma cíclica
    // TBCLR: Neteja i posa a zero el comptador del Timer B1 (reset)
    // Nota: mode CONTINUOUS, sense conflicte amb init_PWM_LED
    TB1CTL   = TBSSEL__SMCLK | ID__8 | MC__CONTINUOUS | TBCLR;// Com que SMCLK/8 = 2MHz, aleshores 1 tick = 0.5us
    
    //Configuració:
    // CM_3: Captura tant le flanc de pujada com de baixada.
    // CCIS_0: Tria la font d'entrada de captura de tipus CCIxA (connectada al pin echo)
    // SCS: Sincronitza la captura amb el rellotge del Timer per evitar problemes de sincronia
    // CAP: Configura el mode Captura
    // CCIE: Habilita la interrupció del mòdul de captura CCR2.
    TB1CCTL2 = CM_3 | CCIS_0 | SCS | CAP | CCIE;
}


void trigger_ultrasons(void) {
    // netejar estat anterior, abans de fer servir el trigger
    __disable_interrupt(); // Desactiva les interrupcions globals per evitar interferències mentre es netegen les dades
    captura_inicio     = 0; // Posar la memòria del moment d'inici del pols a un estat conegut
    captura_fin        = 0; // Posar la memòria del moment final del pols a un estat conegut
    captura_completada = 0; // Posar el flag en un estat conegut(0, fals), preparant el sistema per a una nova captura
    __enable_interrupt(); // Activar les interrupcions globals

    P2OUT |=  BIT3; // Posar el pin trig a HIGH per activar el sensor d'ultrasons
    __delay_cycles(160);// Mantenir el pols en HIGH durant 10 us (10us * 16 cicles per us a 16MHz)
    P2OUT &= ~BIT3; // Posar el pin trig a LOW per finalitzar el senyal d'activació
}

float get_distancia_cm(void) {

    __disable_interrupt();   // Desactiva les interrupcions globals per llegir variables volàtils de la distància
    uint8_t  listo = captura_completada; // Copia l'estat del flag de completat a una variable local
    uint16_t ini   = captura_inicio; // Copia el valor de l'instant inicial a una variable local
    uint16_t fin   = captura_fin; // Copia el valor de l'instant final a una variable local
    if (listo) captura_completada = 0; // Si ja s'ha fet la mesura, alehores, flag es posa a 0 (fals) per començar el següent cicle
    __enable_interrupt();  // Activar les interrupcions globals de nou

    if (!listo) return -1.0f;  // Si encara no s'ha acabat la mesura, ens retorna un valor negatiu, que per nosaltres indicarà error
    
    uint16_t diff;  // Variable local per desar el temps total del pols calculat en cicles del Timer

    if (fin >= ini) // Si el temporitzador no ha desbordat (passat de 0xFFFF a 0x0000) durant la mesura
        diff = fin - ini;  // La diferència és la mesura final i l'inici
    else     // Si el temporitzador s'ha desbordat el comptador ha donat tota la volta durant el pols d'echo
        diff = (0xFFFFu - ini) + fin + 1u; // Calcula els cicles que quedaven per acabar el cicle més els cicles transcorreguts en el nou

    //Com que tenim SMCLK/8, aleshores cada tick és 0.5us, per tal de calcular el valor correcte en cm, cal tenir en compte la velocitat del so
    // velocitat del so 343m/s, aleshores, distancia_cm = diff * 0.5us * 343m/s / 2 = diff * 0.00008575m, llavors, diff * 0.008575cm
    return (float)diff * 0.008575f; //multipliquem pel factor obtingut
}


/********************************************************************/
//INTERRUPCIONS
/********************************************************************/
// Interrupcion del ADC
#pragma vector=ADC_VECTOR //vector interrupció del mòdul ADC
__interrupt void ADC_ISR(void){ // Inici d'interrupció (ISR) de l'ADC

    if (ADCIFG & ADCIFG0) {  // Comprovar si la interrupció ha estat provocada pel canal (ADCMEM0)

        ADCIFG &= ~ADCIFG0; // Netejar el bit del flag d'interrupció, ja que ha estat gestionada

        __bic_SR_register_on_exit(LPM0_bits);  // En sortir de la interrupció, desperta el CPU eliminant el mode de baix consum (LPM0)
    }
}

/********************************************************************/
//Interrupcions del Joystick
#pragma vector=PORT3_VECTOR // Vector d'interrupció dels pins del Port 3 (Joystick)
__interrupt void Port3_ISR(void) // Inici d'interrupció del Port 3
{
    //Amb __even_in_range s'optimitza el codi de salt avaluant el registre numèric d'interrupcions fins al valor màxim (0x10)
    uint8_t iv = __even_in_range(P3IV, 0x10);

    switch (iv) {
        case 0x08: //Correspon a una interrupció al pin P3.3 (Joystick Dreta)
            estado_joystick = (P3IN & BIT3) ? 0 : 1; // Si el pin llegeix 0 (premut), es posa l'estat a 1, si està obert es posa a 0
            break;
        case 0x06:  //Correspon a una interrupció al pin P3.2 (Joystick Esquerra)
            estado_joystick = (P3IN & BIT2) ? 0 : 2;  // Si el pin està premut, l'estat passa a ser 2, altrament torna a 0
            break;
        case 0x0E:  //Correspon a una interrupció al pin P3.6 (Joystick Amunt)
            estado_joystick = (P3IN & BIT6) ? 0 : 4; // Si el pin es prem, assigna el codi d'estat 4, si es deixa anar és 0
            break;
        case 0x10: //Correspon a una interrupció al pin P3.7 (Joystick Avall)
            estado_joystick = (P3IN & BIT7) ? 0 : 4; // Si es prem el polsador d'avall, s'assigna el codi d'estat 4, o 0 si s'allibera
            break;
       case 0x0C: //Correspon a una interrupció al pin P3.5 (Botó de selecció del joystick)
             if (!(P3IN & sel)) {  // Comprovar si realment el polsador "sel" s'ha pulsat (estat baix a causa del Pull-up)
                uint16_t ara = TB1R; // Llegim el comptador lliure del Timer B1 (una vegada ja funcioni el ultrasons), 1 tick=0.5us
                uint16_t transcorregut = ara - ultim_canvi_modo_tb1; // Resta en aritmetica de 16 bits: funciona be encara que hi hagi hagut un overflow pel mig
                // Llindar ~20000 ticks = 10ms per tal d'evitar els rebots del botó del joystick, i tenir un marge d'uns pocs ms
                if (transcorregut > 20000) {
                    // Incrementa per canviar el mode de funcionament del robot: 0-1-2-3-4-0...
                    modo_robot = (modo_robot + 1) % 5; // 0=manual, 1=luz, 2=linea, 3=paret, 4=crossroad,0=manual...
                    estado_joystick = 0; // Netejar el joystick en fer un canvi de mode
                    ultim_canvi_modo_tb1 = ara; // Desar la marca de temps d'aquesta pulsacio acceptada
                }
            }
            break;
        default: break; // Si és qualsevol altre pin del Port 3 no controlat, no fa res i surt
    }

    P3IES ^= (BIT2 | BIT3 | BIT6 | BIT7); // Inverteix el flanc d'interrupció dels pins (si cercava baixada, ara cercarà pujada per detectar quan es deixa anar)
    P3IFG &= ~(BIT2 | BIT3 | BIT5 | BIT6 | BIT7); // Neteja els flags d'interrupció dels pins modificats per tancar el cicle
}

/********************************************************************/
//Interrupcions del timer B1
#pragma vector = TIMER1_B1_VECTOR // Vector d'interrupció del Timer B1
__interrupt void Timer_B1_ISR(void) // Inici d'interrupció del temporitzador governat pel Timer B1 (utilitzat per l'echo del ultrasons)
{
    switch (__even_in_range(TB1IV, 14)) { // S'avalua quin canal del temporitzador ha generat l'avís (fins al valor 14 de registre)

        case 0x04:  // CCR2,Correspon directament al mòdul de captura 2 (Pin P2.1 / echo)
            if (TB1CCTL2 & CCI) { // Comprova l'estat actual de la línia d'entrada (CCI). Si és 1, vol dir que el senyal ha pujat
                // Flanc de pujada: inici del pols echo
                captura_inicio = TB1CCR2;  // Copia el valor del comptador actual a captura_inicio
                captura_completada = 0;  // invalidar medida anterior
            } else { // Si el pin és 0, vol dir que el senyal acaba de baixar (final de echo)
                // Flanc de baixada: fin del pols echo
                captura_fin = TB1CCR2; // Copia el valor del comptador actual a captura_fin
                captura_completada = 1; // Fixa el flag a 1 per avisar que ja pot calcular la distància 
            }
            TB1CCTL2 &= ~CCIFG;  // Neteja flag d'interrupció d'aquest mòdul de captura per permetre futures interrupcions
            break;
        default:
            break; // Ignorar qualsevol altra interrupció de canals no usats pel temporitzador
    }
}
