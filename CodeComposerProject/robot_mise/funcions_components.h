/*
*Arxiu: funcions_components.h
*Autor: Kevin Brandon Ventura Laura, Raúl Casado Luque
*Creat: 05/06/2026
*Descripció: Declaracions de les funcions de control del robot declarades en funcions_components.c
*/
/********************************************************************/ 
//Condicional per la compilació
#ifndef FUNCIONS_COMPONENTS_H_ // Si no s'ha definit abans aquesta capçalera
#define FUNCIONS_COMPONENTS_H_ // Es defineix en aquest arxiu, evita errors ( per exemple, el fitxer s'inclogui múltiples vegades i doni errors de duplicat)

#include <stdint.h> //llibreria
//Definició de periode de PWM del LED
#define PWM_PERIOD 65535 // Defineix el període màxim del Timer per al PWM (valor per a un comptador de 16 bits: 0xFFFF)
// calculat = (percentatge * 65535) / 100 

/********************************************************************/ 

void init_PWM_LED(void);  // Inicialitzar LED governat pel timer B1
void LED_Percentatge(uint8_t percentage);  // Configura el duty cycle (brillantor) del LED passant un valor de 0 a 100%
void LED_PWM(void); // Activar LED de brillantor variable 
void init_ldr(void);  // Inicialitzar els LDRs
uint16_t leer_ADC(uint8_t canal); //Llegir digitalment el valor d'un canal de l'ADC (retorna de 0 a 1023)
uint16_t get_ldr_esq(void);// Retorna la lectura analògica actual del sensor LDR esquerre (Canal A8)
uint16_t get_ldr_dre(void); // Retorna la lectura analògica actual del sensor LDR dret (Canal A9)
uint8_t get_estado_joystick(void); // Retorna quina direcció (estat) del joystick està actualment executant-se
uint8_t get_modo_robot(void); // Retorna el mode de funcionament actual del robot (Manual, Llum, Línia, Paret, crossroad.)
void reset_estado_joystick(void); // Posa a 0 l'estat del joystick (estat conegut)
void init_entradas(void); //Inicialitzar el joystick
void init_ultrasons(void);  //inicialitzar el ultrasons governat pel timer B1
void trigger_ultrasons(void); // Envia el pols elèctric d'activació de 10us al sensor perquè llanci les ones
float get_distancia_cm(void);  // Calcula i retorna la distància  en cm ( -1.0f si no està llesta)

#endif  /* FUNCIONS_COMPONENTS_H_ */ // Final del condicional
