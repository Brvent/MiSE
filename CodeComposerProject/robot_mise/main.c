/*
*Arxiu: robot.c
*Autor: Kevin Brandon Ventura Laura, Raúl Casado Luque
*Creat: 20/06/2026
*Descripció: Implementació de totes les funcions per fer la demo
*/
/********************************************************************/ 
//Llibreries del microcontrolador i mòduls propis i del professor
#include <msp430.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "funcions_micro.h"
#include "funcions_components.h"
#include "robot.h"
#include "AT.h"
#include "recursos.h"
#include "uart_alumnos.h"
/********************************************************************/ 
#define id_robot 0x10 //Adreça del robot
/********************************************************************/ 
//trames del motor, per les direccions del joystick en mode manual
uint8_t motor_up    [5] = {0x00, 1, 50, 1, 50};
uint8_t motor_down  [5] = {0x00, 2, 50, 2, 50};
uint8_t motor_stop  [5] = {0x00, 1,  0, 1,  0};
uint8_t motor_left  [5] = {0x00, 2, 50, 1, 50};
uint8_t motor_right [5] = {0x00, 1, 50, 2, 50};

/********************************************************************/ 
//Funció inicialitzar el clock
void init_clocks(void)
{
    FRCTL0 = FRCTLPW | NWAITS_1; //Desbloquear els registres de la memòria FRAM fem l'ús de la contrasenya (FRCTLPW) i in cicle d'espera de control (N_WAITS_1) per 16MHz
    __bis_SR_register(SCG0); // Activar SCG0 al Registre d'Estat (SR) per apagar temporalment el bucle del FLL i poder configurar el DCO
    CSCTL3 |= SELREF__XT1CLK; // Seleccionar el cristall extern o intern XT1 (32768 kHz) com a font de referència per al modulador FLL
    CSCTL1  = DCORSEL_5; // Configurar el rang de freqüència de l'oscil·lador DCO a 16 MHz
    CSCTL2  = FLLD_0 + 487; // Estableix el divisor del FLL en 1 (FLLD_0) i el multiplicador N en 487 per fixar el rellotge a 16 MHz
    //fmclk = (N+1)fref--> N=fmclk/fref-1= 16000000/32768-1~487  
    __delay_cycles(3); //Esperar 3 cicles de rellotge perquè els canvis s'assentin als registres físics
    __bic_SR_register(SCG0);// Netejar el bit SCG0 al Registre d'Estat per encendre de nou el FLL i deixar que estabilitzi la freqüència 
    CSCTL4  = SELMS__DCOCLKDIV | SELA__XT1CLK; // Configurar MCLK i SMCLK per fer servir el DCO dividit (SELMS) i el rellotge auxiliar ACLK per fer servir el cristall lent XT1 (SELA)
    P1DIR  |= BIT0; // Configurar el pin P1.0 com a sortida
    P1OUT  &= ~BIT0; // Apaga el LED del pin P1.0 posant la seva sortida a nivell LOW
    PM5CTL0 &= ~LOCKLPM5; //Baix consum
}
/********************************************************************/ 
//funció per processar trama rebuda per wifi i fer que el robot rebi la instrucció
static void processar_modul_wifi(struct RxATReturn *cmd)
{
    if (cmd->num_bytes < 6) return; // Si el paquet rebut té menys de 6 bytes, està incomplet (invàlid) i finalitza la funció
    if (cmd->StatusPacket[0] != 0xFF) return; // Verifica que el primer byte de la capçalera sigui 0xFF; si no ho és, el descarta
    if (cmd->StatusPacket[1] != 0xFF) return; // Verifica que el segon byte de la capçalera sigui també 0xFF. Si falla, surt de la funció

    uint8_t id_mod  = cmd->StatusPacket[2]; // Extraer de l'estructura l'ID del mòdul de destí (motor,LEDs, etc)
    uint8_t instruc = cmd->StatusPacket[4]; // Extraer el tipus d'instrucció que se sol·licita 
    uint8_t address = cmd->StatusPacket[5];  // Extraer l'adreça del registre de memòria interna del robot que es vol modificar

    if (instruc != INSTR_WRITE) return; // Si la instrucció no és una ordre d'escriptura (INSTR_WRITE), el robot no ha de modificar res i surt
    // Si s'envia instrucció als registre del LED
    if (address == Led) { 
        uint8_t color = cmd->StatusPacket[6]; // Extraer el byte que defineix el color per al LED
        uint8_t led_tram[3] = {CMD_RGB, color, color};// Preparar la trama de dades per enviar 
        I2C_send(id_robot, led_tram, 3);   // Enviar el paquet de 3 bytes
    }
    //Si s'envia instrucció als registre del motor
    else if (address == Motor) {
        uint8_t vel     = cmd->StatusPacket[6];  // Extraer el byte de velocitat del paquet rebut
        uint8_t sentido = cmd->StatusPacket[7];  // Extraer el byte que defineix el sentit del moviment del motor
        uint8_t mot[5]  = {CMD_MOTOR, sentido, vel, sentido, vel}; // Preparar una trama per enviar

        if      (id_mod == ID_Left)  { mot[3] = 1; mot[4] = 0; }   // Si només ens dirigim al motor esquerre
        else if (id_mod == ID_Right) { mot[1] = 1; mot[2] = 0; }   // Si només ens dirigim al motor dret

        I2C_send(id_robot, mot, 5);  // Enviar el paquet de 5 bytes

    }
    //EStat del paquet
    uint8_t error  = 0x00;  // Defineix el codi d'error com a 0x00 (Tot correcte / Sense errors)
    uint8_t length = 2;  //legth ens indica els bytes restants: el byte d'error + el byte de checksum
    uint8_t chk    = ~(id_mod + length + error) & 0xFF;     // Calcular el Checksum Bioloid aplicant l'operació NOT (~) a la suma d'ID + LENGTH + ERROR i aplicant una màscara de 8 bits
    uint8_t status_packet[6] = {0xFF, 0xFF, id_mod, length, error, chk};  // Crea la matriu del paquet d'estat amb el protocol Bioloid: [0xFF, 0xFF, ID, Longitud, Error, Checksum]
    TxPacket(sizeof(status_packet), status_packet);     // Transmet el paquet d'estat de tornada a l'ordinador emissor fent servir la funció de la UART d'alumnes

}

/********************************************************************/ 
//Main

int main(void)
{
    /********************************************************************/ 
    //Inicialitzar funcions
    WDTCTL = WDTPW | WDTHOLD; // stop watchdog timer
    init_TimerB0(); //Inicialitzar timerB0
    init_clocks(); //Inicialitzar rellotges
    i2c_init(); //Inicialitzar I2C
    init_entradas(); //Iniciatlitzar Joystick
    init_ultrasons(); //Inicialitzat Ultrasons
    init_ldr(); //Inicialitzar LDRs
    init_PWM_LED(); //Inicialitzar LED controlat per PWM
    __enable_interrupt(); //Habilitar interrupcions globals
    init_LCD(); //Inicialitzar LCD
    init_uart_wifi();//Iinicialitzar mòdul WIFI
    delay_ms(2000);  //Esperar resposta del mòdul WIFI (ESP-01)
    /********************************************************************/
    //Comprovar el correcte funcionament del mòdul WIFI, mitjançant el LCD
    LCD_send_words("WiFi...", NULL);
    if (comando_AT()) {
        LCD_send_words("WiFi OK", NULL);
        leds_verd();
    } else {
        LCD_send_words("WiFi FAIL", NULL);
        leds_vermell();
    }
    delay_ms(2000); //mostrar per pantalla durant 2s

    // mostrar IP en LCD
    uint8_t ip_buf[64] = {0};
    LCD_clear();
    if (get_IP(ip_buf) == 0) {
        LCD_send_words("IP:", (char*)ip_buf);
        delay_ms(4000); //mostrar per pantalla 4s
    }
    LCD_clear(); //netejar LCD
    /********************************************************************/
    //DEMO
    delay_ms(1000);//esperar
    LCD_send_words("Preparando", "Demo"); //mostrar per pantalla del LCD
    delay_ms(2000);//mostrar per pantalla 2s
    LCD_clear();  //netejar LCD
    delay_ms(1000);//esperar
    /********************************************************************/
    //comprovar els LEDs
    leds_vermell();
    delay_ms(500);
    leds_verd();
    delay_ms(500); 
    leds_groc();
    delay_ms(500);
    leds_blau();
    delay_ms(500);
    leds_fucsia();
    delay_ms(500);
    leds_celeste();
    delay_ms(500);
    leds_blanc();
    delay_ms(500);
    random_leds();
    delay_ms(500);
    leds_off();
    delay_ms(500);
    /********************************************************************/
    // Comprovació de funcionament de motors
    endavant_robot(50);
    delay_ms(1000);
    enrere_robot(50);
    delay_ms(1000);
    girar_antihorari(50);
    delay_ms(1000);
    girar_horari(50);
    delay_ms(1000);
    aturar_robot();
    /********************************************************************/
    //Entrando a bucle main
    LCD_send_words("Main", NULL);
    delay_ms(1000);
    LCD_clear();
    /********************************************************************/
    //variables de bucle principal
    uint16_t luz_esq = 0, luz_dre = 0; //emmagatzemen el valor dels LDRs (valor de lluminositat capturada)
    int16_t  dif_luz = 0; //emmagatzemen la diferència de llum entre els LDRs
    static uint8_t linetrack_calibrado = 0; // Indicador d'estat que comprova si s'ha fet el calibratge inicial dels sensors de línia del robot
    static uint8_t modo_anterior = 0xFF; // Estat del mode de l'execució prèvia per detectar canvis d'estat
    static uint8_t joy_anterior  = 0xFF; // Estat anterior del Joystick 
    /********************************************************************/
    //Valor virtual de joystick cap a l'esquerra
    //Variables auxiliar per solucionar el problema de hardware, no funciona la direcció esquerra del joystick (la connexió de pistes no hi ha problema, per tant, segurament sigui intern)
    static const uint8_t SECUENCIA[5] = {1, 2, 4, 1, 2}; //seqüència per fer servir la direcció esquerra (up,down,right,up,down) y després up es cap a l'esquerra
    static uint8_t seq_paso   = 0; //variable static per recordar la seqüència
    static uint8_t left_actiu = 0; // Variable flag que indica que la seqüència s'ha donat amb èxit i activa el motor cap a left
    while (1) {
        LED_PWM(); // Activar el LED de brinllantor controlat per un PWM
        uint8_t modo_actual = get_modo_robot(); //  Estat del mode del robot actualment
         /********************************************************************/
        //detecció de canvi de mode
        if (modo_actual != modo_anterior) { //si es canvia el mode (pulsant el joystick)
            I2C_send(id_robot, motor_stop, 5); // Aturar el robot abans de canviar d'algorisme
            delay_ms(200); // Pausa de seguretat
            modo_anterior = modo_actual; // Desar el nou mode com a referència del passat
            joy_anterior  = 0xFF; //Reiniciar la memòria del joystick perquè agafi la nova ordre

            //mostrar el mode actual per pantalla
            switch (modo_actual) {
                case 0: 
                    LCD_send_words("Manual", NULL);
                    delay_ms(4000);
                    LCD_clear();
                    break;
                case 1:
                    LCD_send_words("llum", NULL);
                    delay_ms(4000);
                    LCD_clear();
                    break;
                case 2: 
                    LCD_send_words("linia", NULL);
                    delay_ms(4000);
                    LCD_clear();
                    break;
                case 3:
                    LCD_send_words("paret", NULL);
                    delay_ms(4000);
                    LCD_clear();
                    break;
                case 4:
                    LCD_send_words("crossroad", NULL);
                    delay_ms(4000);
                    LCD_clear();
                    break;
            }
            // Si s'entra en qualsevol dels modes (luz=1,linea=2,paret=3,crossroad=3) i és el primer cop:
            if ((modo_actual == 2 || modo_actual == 3 || modo_actual == 4) 
                && !linetrack_calibrado) { 
                I2C_send(id_robot, motor_stop, 5); // El robot és immòbil a la línia de sortida
                delay_ms(4000); // 4 segons per deixar el robot a terra i calibrar el fons
                linetrack_calibrado = 1; // Calibratge de línia completat
            }
        }
        /********************************************************************/
        //Llegir el ultrasons, aquest sensor funciona en tots els modes de funcionament per assegurar-nos que el robot s'aturi si hi ha un obstacle
        trigger_ultrasons(); // Enviar el pols (Trigger) per iniciar la mesura
        uint16_t timeout = 0; //variable comptador de temps d'espera local
        float distancia_cm = -1.0f; // Posar la distància a un valor conegut (error incial) 
        while (timeout < 60) {  // Bucle d'espera fins que hi hagi dades de l'eco
            delay_ms(1); //esperar
            distancia_cm = get_distancia_cm(); //actualitzar el valor de la distància
            if (distancia_cm >= 0.0f) break;  // Si la distància obtinguda és vàlida (major o igual a zero), trenca l'espera
            timeout++; // Incrementa el comptador de timeout per evitar que el micro es quedi penjat
        }
        /********************************************************************/
       //mode manual, WIFI i Joystick
       
        if (modo_actual == 0) { //mode manual
            // Algorisme de Seguretat Anti-col·lisió (prioritat) 
             if (distancia_cm > 0.5f && distancia_cm < 15.0f) {// Si el sensor ultrasònic detecta un objecte real a menys de 15 cm de distància d'aquesta distància atura els motors
                I2C_send(id_robot, motor_stop, 5);// Atura els motors per evitar xocar
            } else {// Si el camí està lliure s'executa el corresponent
            //Control de joystick
            uint8_t joy_actual = get_estado_joystick();// Consultar l'estat de la direcció que pren l'usuari
            if (joy_actual != joy_anterior) { // Si la direcció del joystick ha canviat respecte l'últim cicle:
                joy_anterior = joy_actual; // Desar la direcció actual

                //Valor virtual cap a l'esquerra
                if (joy_actual != 0) { //veure l'estat quan es mou
                    if (joy_actual == SECUENCIA[seq_paso]) { //comprovar si la seqüència s'ha fet, si s'ha fet comprovar que el nombre de moviments es correcte
                        seq_paso++;  // Si coincideix incrementa el comptador del patró, en aquest cas ha d'esperar 5 moviments
                        if (seq_paso >= 5) {  //Si s'ha completat els 5 moviments,
                            seq_paso = 0; //reinicia la seq_paso quan es vulgui utilitzar de nou el moviment cap a l'esquerra 
                            left_actiu = 1; // Activar el Flag, el moviment cap a l'esquerra està disponible
                        }  
                    } else {  // Si es fa un moviment que no coincideix amb la seqüència
                        seq_paso = (joy_actual == SECUENCIA[0]) ? 1 : 0; //si el moviment es incorrecte al primer pas desa un 1 (avança al següent moviment ) i si és qualsevol altre cosa posa a 0
                    }
                }
                if (joy_actual == 1 && left_actiu) {  //Si la seqüència ja estava completada (left_actiu == 1) i es mou el joystick cap amunt 
                    joy_actual = 3; //Intercepta el valor i el força a '3', per tant, tindriem el valor virtualment a "Esquerra"
                    left_actiu = 0;  //Desactivar el flag, per quitar el valor virtual
                    seq_paso = 0; //posar el comptador de moviments a 0
                }

                switch (joy_actual) {  // Executa l'acció mecànica i escriu la comanda al LCD
                    case 1: 
                        I2C_send(id_robot, motor_up,5); //up
                        LCD_send_words("UP", NULL); //enviar direcció al LCD
                         break;
                    case 2: 
                        I2C_send(id_robot, motor_down, 5); //down
                        LCD_send_words("DOWN", NULL); //enviar direcció al LCD
                        break;
                    case 3:
                        I2C_send(id_robot, motor_left,  5); //left
                        LCD_send_words("LEFT", NULL); //enviar direcció al LCD 
                        break;
                    case 4:
                        I2C_send(id_robot, motor_right, 5);//right
                        LCD_send_words("RIGHT", NULL);//enviar direcció al LCD
                        break;
                    default: 
                        I2C_send(id_robot, motor_stop, 5); //stop
                        LCD_clear();//enviar direcció al LCD
                        break;
                }
            }
            //comprovar que tots els pins estan a nivell HIGH, si es així, el joystick ha tornat a estar en repòs
            if ((P3IN & (BIT6|BIT7|BIT2|BIT3)) == (BIT6|BIT7|BIT2|BIT3)) {
                reset_estado_joystick(); // Netejar l'estat del joystick
            }

            //ontrol paral·lel de mòdul WiFi (només operatiu mode manual) 
            struct RxATReturn cmd = recibir_wifi(); // Interroga el buffer de recepció de la UART WiFi
            if (cmd.num_bytes > 0) {// Si hi ha algun paquet de dades nou que s'ha descarregat
                processar_modul_wifi(&cmd); // Processa i executa l'ordre continguda en la trama 
            }
        }
        }
        /********************************************************************/
        //Modes automàtics:escapar de llum, seguidor de línia, seguidor de paret i crossroad
        else {
            if (modo_actual == 1) {  // mode escapar de Llum, captura de valors
                luz_esq = get_ldr_esq(); // Captura el valor de la intensitat de llum de l'esquerra
                luz_dre = get_ldr_dre(); // Captura el valor de la intensitat de llum de la dreta
                dif_luz = (int16_t)luz_esq - (int16_t)luz_dre; // Calcula la diferència de llum entre els costats
            }
            // Algorisme de Seguretat Anti-col·lisió (prioritat) 
            if (distancia_cm > 0.5f && distancia_cm < 15.0f) { // Si el sensor ultrasònic detecta un objecte real a menys de 15 cm de distància d'aquesta distància atura els motors
                I2C_send(id_robot, motor_stop, 5); // Atura els motors per evitar xocar
            } else {// Si el camí està lliure s'executa el corresponent

                if (modo_actual == 1) {  // mode escapar de Llum 
                    int16_t umbral = 100; // Tolerància per evitar girs constants per petites variacions de soroll
                    if(dif_luz >  umbral){
                        I2C_send(id_robot, motor_left,  5);// Hi ha molta més llum a l'esquerra, gira cap allà
                    }else if (dif_luz < -umbral){
                         I2C_send(id_robot, motor_right, 5);// Hi ha molta més llum a la dreta, gira cap allà
                    }else{
                        I2C_send(id_robot, motor_up,    5);// Llum equilibrada, avança de dret cap a la font
                    }
                }
                else if (modo_actual == 2){
                    linetrack(0, 100); //mode seguidor de línia
                }else if (modo_actual == 3){
                    linetrack(1, 55); //mode seguidor de paret
                }else if (modo_actual == 4){
                    linetrack(2, 80);//mode crossroad
                }
            }
        }

        delay_ms(2); //Espera
    }
}


