/*
*Arxiu: funcions_micro.h
*Autor: Kevin Brandon Ventura Laura, Raúl Casado Luque
*Creat: 05/06/2026
*Descripció: Declaracions de les funcions del microcontrolador de gestió, declarades en funcions_micro.c
*/
/********************************************************************/ 
//Condicional per la compilació
#ifndef FUNCIONS_MICRO_H_ // Si no s'ha definit abans aquesta capçalera
#define FUNCIONS_MICRO_H_ // Es defineix en aquest arxiu, evita errors ( per exemple, el fitxer s'inclogui múltiples vegades i doni errors de duplicat)
/********************************************************************/ 
void init_TimerB0(void);  // Inicialitzar timer B0 
void delay_ms(uint16_t ms); // funció de retard per al temps d'espera(durant el temps indicat en mil·lisegons) necessari en algunes funcions.
void i2c_init(); //Inicialitzar I2C
void I2C_send(uint8_t addr, uint8_t *buffer, uint8_t n_dades); //Enviar bytes per I2C cap a un dispositiu esclau
void I2C_receive(uint8_t addr, uint8_t *buffer, uint8_t n_dades); // Rebre dades per I2C d'un dispositiu i desar al buffer
#endif /* FUNCIONS_MICRO_H_ */ // Final del condicional
 

