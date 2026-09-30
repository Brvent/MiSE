/*
*Arxiu: robot.c
*Autor: Kevin Brandon Ventura Laura, Raúl Casado Luque
*Creat: 05/06/2026
*Descripció:
*/
/********************************************************************/ 
//Llibreries del microcontrolador i mòdul propi
#include <msp430.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "funcions_micro.h"

/********************************************************************/
//variables de los LEDs per definir els colors
// off=0,vermell=1,verd=2,groc=3,blau=4,fucsia=5,celeste=6,blanc=7
#define off 0
#define vermell 1
#define verd 2
#define groc 3
#define blau 4
#define fucsia 5
#define celeste 6
#define blanc 7
#define id_robot 0x10 // Adreça del robot
#define led_add 0x0B // Adreça dels LEDs RGB del robot
/********************************************************************/
//variables dels motors 
#define motor_add 0x00 // Adreça dels motors
#define fdir 1 //Direcció de moviment frontal
#define ddir 2 //Direcció de moviment Darrere
#define aturar 0 //Aturar el moviment
/********************************************************************/
//variables del lcd
#define lcd_addr 0x3E //Adreça del LCD
/********************************************************************/
//variables del linetrack
#define linetrack_addr 0x1D //Adreça del linetrack
/********************************************************************/
//Trames dels LEDS on els dos tenen el mateix comportament
uint8_t led_off_tram[3] = {led_add,off,off};
uint8_t led_vermell_tram[3] = {led_add,vermell,vermell};
uint8_t led_verd_tram[3] = {led_add, verd, verd};
uint8_t led_groc_tram[3] = {led_add, groc, groc};
uint8_t led_blau_tram[3] ={led_add, blau, blau};
uint8_t led_fucsia_tram[3] ={led_add, fucsia, fucsia};
uint8_t led_celeste_tram[3] = {led_add, celeste, celeste};
uint8_t led_blanc_tram[3] = {led_add, blanc, blanc};
/********************************************************************/
//trames del LCD
uint8_t lcd_trama_ini[8] = {0x00, 0x39, 0x14, 0x74, 0x54, 0x6F, 0x0C, 0x01}; //trama inicialitzacio 3V (pag 9 datasheet del LCD)
uint8_t lcd_clear[2] = {0x00, 0x01}; //Netejar pantalla del LCD
 
/********************************************************************/
//tramea del motor
uint8_t motor_tram[5];
/********************************************************************/
//trama del linetrack
uint8_t linetrack_reg[6];
/********************************************************************/
// Funcions dels LEDs
//nota: delay_ms(1) dura 1ms

void leds_off(void){  //Apagar els LEDs
    delay_ms(1);
    I2C_send(id_robot, led_off_tram, 3);
}

void leds_vermell(void){ //Posar en vermell els LEDs
    delay_ms(1);
    I2C_send(id_robot, led_vermell_tram, 3);
}

void leds_verd(void){  //Posar en verd els LEDs
    delay_ms(1);
    I2C_send(id_robot, led_verd_tram, 3);
}

void leds_groc(void){ //Posar en groc els LEDs
    delay_ms(1);
    I2C_send(id_robot, led_groc_tram, 3);
}

void leds_blau(void){ //Posar en blau els LEDs
    delay_ms(1);
    I2C_send(id_robot, led_blau_tram, 3);
}

void leds_fucsia(void){ //Posar en fucsia els LEDs
    delay_ms(1);
    I2C_send(id_robot, led_fucsia_tram, 3);
}

void leds_celeste(void){ //Posar en celeste els LEDs
    delay_ms(1);
    I2C_send(id_robot, led_celeste_tram, 3);
}

void leds_blanc(void){ //Posar en blanc els LEDs
    delay_ms(1);
    I2C_send(id_robot, led_blanc_tram, 3);
}

void random_leds(void){ //Posar els LED a un color random
    uint8_t random_buffer[3]; //longitud de trama
    random_buffer[0] = led_add; // Adreça dels LEDs RGB del robot
    random_buffer[1] = (rand() % 7) + 1;  //Color random entre els 7 que existeixen
    random_buffer[2] = (rand() % 7) + 1; 
    delay_ms(1); //espera
    I2C_send(id_robot, random_buffer, 3); //enviar
}
/********************************************************************/
// Funcions per controlar el moviment dels motors per implementar en el linetrack i els seus modes de funcionament
//nota: delay_ms(1) dura 1ms

void aturar_robot(void){ //Aturar el robot
    motor_tram[0] = motor_add;
    motor_tram[1] = fdir;
    motor_tram[2] = aturar;
    motor_tram[3] = fdir;
    motor_tram[4] = aturar;
    I2C_send(id_robot, motor_tram, sizeof(motor_tram));
}

void endavant_robot(uint8_t vel_end){ //Moure endavant el robot
    motor_tram[0] = motor_add;
    motor_tram[1] = fdir;
    motor_tram[2] = vel_end;
    motor_tram[3] = fdir;
    motor_tram[4] = vel_end;
    I2C_send(id_robot, motor_tram, sizeof(motor_tram));
}

void enrere_robot(uint8_t vel_enr){ //Moure enrere el robot
    motor_tram[0] = motor_add;
    motor_tram[1] = ddir;
    motor_tram[2] = vel_enr;
    motor_tram[3] = ddir;
    motor_tram[4] = vel_enr;
    I2C_send(id_robot, motor_tram, sizeof(motor_tram));
}

void endavant_girar_esq(uint8_t vel_end_gir){ //Moure endavant i girar cap a l'esquerra el robot
    motor_tram[0] = motor_add;
    motor_tram[1] = fdir;
    motor_tram[2] = aturar;
    motor_tram[3] = fdir;
    motor_tram[4] = vel_end_gir;
    I2C_send(id_robot, motor_tram, sizeof(motor_tram));
}

void enrere_girar_esq(uint8_t vel_enr_gir){//Moure enrere i girar cap a l'esquerra el robot
    motor_tram[0] = motor_add;
    motor_tram[1] = fdir;
    motor_tram[2] = aturar;
    motor_tram[3] = ddir;
    motor_tram[4] = vel_enr_gir;
    I2C_send(id_robot, motor_tram, sizeof(motor_tram));
}

void girar_antihorari(uint8_t vel_antiho){ //Girar el robot en sentit antihorari
    motor_tram[0] = motor_add;
    motor_tram[1] = ddir;
    motor_tram[2] = vel_antiho;
    motor_tram[3] = fdir;
    motor_tram[4] = vel_antiho;
    I2C_send(id_robot, motor_tram, sizeof(motor_tram));
}

void endavant_girar_dr(uint8_t vel_end_gir_dr){//Moure endavant i girar cap a la dreta el robot
    motor_tram[0] = motor_add;
    motor_tram[1] = ddir;
    motor_tram[2] = vel_end_gir_dr;
    motor_tram[3] = ddir;
    motor_tram[4] = aturar;
    I2C_send(id_robot, motor_tram, sizeof(motor_tram));
}

void enrere_girar_dr(uint8_t vel_enr_gir_dr){ //Moure enrere i girar cap a la dreta el robot
    motor_tram[0] = motor_add;
    motor_tram[1] = ddir;
    motor_tram[2] = aturar;
    motor_tram[3] = fdir;
    motor_tram[4] = vel_enr_gir_dr;
    I2C_send(id_robot, motor_tram, sizeof(motor_tram));
}

void girar_horari(uint8_t vel_ho){ //Girar el robot en sentit horari
    motor_tram[0] = motor_add;
    motor_tram[1] = fdir;
    motor_tram[2] = vel_ho;
    motor_tram[3] = ddir;
    motor_tram[4] = vel_ho;
    I2C_send(id_robot, motor_tram, sizeof(motor_tram));
}
/********************************************************************/
//Inicialitzar pantalla LCD (3.3V)
void init_LCD(void){
    P5DIR |= BIT2; // Configurar com a sortida 
    P5OUT &= ~(BIT2); // Posar a LOW, pel reset del LCD 
    delay_ms(10);// Mantenir el reset durant 10 ms 
    P5OUT |= BIT2;// Posar a HIGH per alliberar el LCD del reset i activar-lo
    delay_ms(10);// Mantenir durant 10 ms fins que LCD estigui preparat
    I2C_send(lcd_addr, lcd_trama_ini, sizeof(lcd_trama_ini)); //Enviar trama de inicialització
    delay_ms(10);// Mantenir durant 10 ms fins que LCD s'hagi inicialitzat
}
//Reset la pantalla LCD
void LCD_reset(void){
    P5OUT &= ~(BIT2); // Posar a LOW, pel reset del LCD 
    delay_ms(10);  // Esperar 10 mil·lisegons
    P5OUT |= BIT2; // Posar a HIGH per desactivar el reset per reiniciar la pantalla
    delay_ms(10); //Esperar 10ms fins per deixar que LCD estigui preparat
}
//Netejar pantalla LCD
void LCD_clear(void){
    delay_ms(10); // Esperar 10 mil·lisegons per evitar errors de transmissió 
    I2C_send(lcd_addr, lcd_clear, sizeof(lcd_clear)); //Enviar trama de neteja

}

//Funció per enviar paraules completes a la pantalla, ocupant les dos posicions
void LCD_send_words(char* paraula_dalt, char* paraula_baix) {
    uint8_t cadena[16]; // Crear variable per emmagatzemar la cadena de text a enviar al LCD 
    uint8_t paraula; // Variable per desar la longitud real del text generat per sprintf
    uint8_t cursor_segona_linea[2] = {0x00, 0xC0}; //Trama que permet moure el cursor a la línia 2 de la pantalla

    //Línia de dalt
    if (paraula_dalt != NULL) {  // Si la cadena de text vàlida per a la primera línia
        // Ajuntem el caràcter de control '@' amb la paraula que ens han passat
        paraula = sprintf((char*)cadena, "@%s", paraula_dalt); // Formata la cadena de text amb caràcter '@'  abans del text i ho guarda a 'cadena'
        I2C_send(lcd_addr, cadena, paraula); // Envia la cadena formatada a la pantalla
        delay_ms(20);  // Esperar 20 mil·lisegons perquè la pantalla acabi de processar el text
    }

    //Línia de baix
    if (paraula_baix != NULL) {  // Si s'ha passat una cadena de text vàlida per a la segona línia
        I2C_send(lcd_addr, cursor_segona_linea, 2); //Enviar trama per posar el cursor a la segona línia
        delay_ms(20);// Esperar 20 mil·lisegons per moure el cursor de la pantalla 
        //Una vegada tenim això es pot repetir el mateix que en la primera paraula
        paraula = sprintf((char*)cadena, "@%s", paraula_baix); //Paraula formatada 
        I2C_send(lcd_addr, cadena, paraula); //Enviar segona paraula
        delay_ms(20);// Esperar 20 mil·lisegons perquè la pantalla acabi de processar el text
    }
}


/********************************************************************/
//Funció linetrack, amb tres modes de funcionament
void linetrack(uint8_t mode, uint8_t vel) {

    uint8_t tx_buf[1] = {linetrack_addr};  //  Buffer de transmissió, és l'adreça del registre intern que volem llegir (fotodetectors)
    uint8_t rx_buf[1] = {0x00};             //  Buffer de recepció, variable on el sensor abocarà la dada dels fotodetectors

    // Enviar la peticio de mesura al registre del modul line-track
    I2C_send(id_robot, tx_buf, sizeof(tx_buf)); //Enviar trama per consultar la mesura del fotodetectors (llegir)
    // Rebre i desar la resposta
    I2C_receive(id_robot, rx_buf, sizeof(rx_buf)); //Rep el byte de informació de l'estat del fotodetectors de línia i desar a la a rx_buf
    //Desempaquetant la trama dels 6 sensors del fotodetectors que tenim, mitjançant l'aplicació d'una mascara de bit i desplaçament
    linetrack_reg[0] = rx_buf[0] & ((uint8_t)BIT0);        // Bit 0: Sensor extrem dret (R3)
    linetrack_reg[1] = (rx_buf[0] & ((uint8_t)BIT1)) >> 1; // Bit 1: Sensor intermedi esquerre (R2)
    linetrack_reg[2] = (rx_buf[0] & ((uint8_t)BIT2)) >> 2; // Bit 2: Sensor central dret (R1)
    linetrack_reg[3] = (rx_buf[0] & ((uint8_t)BIT3)) >> 3; // Bit 3: Sensor central esquerre (L1)
    linetrack_reg[4] = (rx_buf[0] & ((uint8_t)BIT4)) >> 4; // Bit 4: Sensor intermedi esquerre (L2)
    linetrack_reg[5] = (rx_buf[0] & ((uint8_t)BIT5)) >> 5; // Bit 5: Sensor extrem esquerre (L3)
    
    //mode de funcionament
    switch(mode){

    case 0: //seguiment de linia 
        if(linetrack_reg[0]){ // Detecció de molt cap a la dreta, fora de la línia
            girar_horari(vel); //rotar en sentit horari
        } else if(linetrack_reg[5]){ //Detecció de molt cap a l'esquerra, fora de la línia
            girar_antihorari(vel);//rotar en sentit horari
        } else if(linetrack_reg[1] || linetrack_reg[2]){ //Detecció d'un limit dret de la línia
            endavant_girar_esq(vel); //endevant i girar cap a l'esquerra
        } else if(linetrack_reg[3] || linetrack_reg[4]){ //Detecció d'un limit esquerra de la línia
            endavant_robot(vel); //endavant robot
            //endavant_girar_dr(vel); //endavant i girar cap a la dreta
        } 
           else{ //Si estem dins dels límits de la línia
            endavant_robot(vel); //endavant robot
        }
        break;

    case 1: //Seguiment de paret 
        if(linetrack_reg[5] && linetrack_reg[4] && linetrack_reg[3] && linetrack_reg[2] && linetrack_reg[1] && linetrack_reg[0]){ //El cami no té una sortida
            /* s'ha arribat a un cami sense sortida --> aturem el robot*/
            aturar_robot(); //Aturar el robot
        } else if(linetrack_reg[4] && linetrack_reg[3] && linetrack_reg[2] && linetrack_reg[1]){ //Pared frontal
            aturar_robot(); //aturar robot
            delay_ms(10); //Esperar 5 ms per tomar la decisió 
            girar_horari(vel); //girar en sentit horari
            delay_ms(10);  //Esperar 5 ms, abans d'aturar
            aturar_robot(); //aturar robot
        } else if(linetrack_reg[1] && linetrack_reg[0]){ // Cantonada dreta
            delay_ms(10);//Esperar 5 ms per tomar la decisió 
            girar_horari(vel);//girar en sentit horari
            delay_ms(10);//Esperar 5 ms
        } else if(linetrack_reg[5] && linetrack_reg[4]){  //Cantonada esquerra
            delay_ms(10);//Esperar 5 ms per tomar la decisió 
            girar_antihorari(vel);//girar en sentit antihorari
            delay_ms(10);//Esperar 5 ms
        } else if(linetrack_reg[0]){ //Paret lateral a la dreta
            /* s'ha detectat una paret lateral a l'esquerra --> gira cap a la dreta */
            delay_ms(10);//Esperar 5 ms per tomar la decisió 
            endavant_girar_dr(vel); //endavant i girar cap a la dreta
            delay_ms(10);//Esperar 5 ms
        } else if(linetrack_reg[5]){//Paret lateral a l'esquerra/dreta?
            delay_ms(10);//Esperar 5 ms per tomar la decisió 
            endavant_girar_esq(vel); //endavant i girar cap a l'esquerra
            delay_ms(10);//Esperar 5 ms
        } else{//si no detectem parets, s'ha d'avançar cap a endavant
            endavant_robot(vel);
        }
        break;

    case 2: // Crossroad 
            
            if (linetrack_reg[4] && linetrack_reg[1]) { // L2 i R2  detecten línia negra
                girar_horari(vel);// Creuament o final de carrer detectat (L2 i R2 actius), girar sobre si mateix 
            } 
            
            else if (!linetrack_reg[3] && !linetrack_reg[2]) { //El robot s'ha sortit de la línia (L1 i R1 no detecten la línia)
                girar_horari(vel); //Cap dels sensors centrals (L1, R1) detecta línia, robot fora de pista, aleshores girar
            } 
            
            // 3. Seguiment de línia normal (L1 i R1 guien el robot)
            else if (linetrack_reg[3] && linetrack_reg[2]) { // Seguiment de línia normal (L1 i R1 guien el robot)
                endavant_robot(vel); //Ambdós sensors centrals en línia, endavant robot
            } 
            else if (linetrack_reg[3] && !linetrack_reg[2]) { // Només el sensor central esquerre (L1) detecta línia 
                endavant_girar_esq(vel); //Endavant i girar cap a l'esquerra 
            } 
            else if (!linetrack_reg[3] && linetrack_reg[2]) {// Només el sensor central esquerre (R1) detecta línia 
                endavant_girar_dr(vel);//Endavant i girar cap a la dreta
            }
            break;
        }
}



