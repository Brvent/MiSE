/*
*Arxiu: robot.h
*Autor: Kevin Brandon Ventura Laura, Raúl Casado Luque
*Creat: 05/06/2026
*Descripció: funcions per implementar components del robot i LCD extern, declarades en robot.c
*/
/********************************************************************/ 
//Condicional per la compilació
#ifndef ROBOT_H_ // Si no s'ha definit abans aquesta capçalera
#define ROBOT_H_// Es defineix en aquest arxiu, evita errors ( per exemple, el fitxer s'inclogui múltiples vegades i doni errors de duplicat)

//Funcions de colors dels LEDs
void leds_off(void);
void leds_vermell(void);
void leds_verd(void);
void leds_groc(void);
void leds_blau(void);
void leds_fucsia(void);
void leds_celeste(void);
void leds_blanc(void);
void random_leds(void);
//Funcions de moviment del motors
void aturar_robot(void);
void endavant_robot(uint8_t vel_end);
void enrere_robot(uint8_t vel_enr);
void endavant_girar_esq(uint8_t vel_end_gir);
void enrere_girar_esq(uint8_t vel_enr_gir);
void girar_antihorari(uint8_t vel_antiho);
void endavant_girar_dr(uint8_t vel_end_gir_dr);
void enrere_girar_dr(uint8_t vel_enr_gir_dr);
void girar_horari(uint8_t vel_ho);
//Funcions del LCD
void init_LCD(void);
void LCD_reset(void);
void LCD_clear(void);
void LCD_send_words(char* paraula_dalt, char* paraula_baix);
//linetrack amb 3 modes de funcionament
void linetrack(uint8_t mode, uint8_t linetrack_vel);
#endif /* ROBOT_H_ */ // Final del condicional