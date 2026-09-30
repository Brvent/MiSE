/*
 * recursos.h
 *
 *  Created on: 01/2021
 *      Author: C. Serre, UB.
 */

#ifndef RECURSOS_H_
#define RECURSOS_H_

#include <stdint.h>

#define PLACA_TEST 1 //1 si se usa la placa verde de test.
					// 0, o Comentar si se usa la placa "normal"

//Recursos de la uart backchannel:
/*UCA1, P4.2 = RXD, P4.3 = TXD */
#define Port_UARTx_SEL P4SEL0
#define BIT_UARTx_RXD BIT2
#define BIT_UARTx_TXD BIT3
#define UCAxTXBUF UCA1TXBUF
#define UCAxRXBUF UCA1RXBUF
#define UCAxIFG UCA1IFG
#define UCAxIE UCA1IE
#define UCAxCTLW0 UCA1CTLW0
#define UCAxSTATW UCA1STATW
#define UCAxBRW UCA1BRW
#define UCAxMCTLW UCA1MCTLW
#define TXDx_READY (UCAxIFG & UCTXIFG)
#define UCAxBusy (UCAxSTATW&UCBUSY)


//La uart para el modulo wifi:
/*UCA0,  P1.6 = RXD, P1.7 = TXD */
#define Port_UARTw_SEL P1SEL0
#define BIT_UARTw_RXD BIT6
#define BIT_UARTw_TXD BIT7
#define UCAwTXBUF UCA0TXBUF
#define UCAwRXBUF UCA0RXBUF
#define UCAwIFG UCA0IFG
#define UCAwIE UCA0IE
#define UCAwCTLW0 UCA0CTLW0
#define UCAwSTATW UCA0STATW
#define UCAwBRW UCA0BRW
#define UCAwMCTLW UCA0MCTLW
#define TXDw_READY (UCAw0IFG & UCTXIFG)
#define UCAwBusy (UCAwSTATW&UCBUSY)

//Recursos para la pantallita LCD:
#define Port_reset_LCD_DIR P5DIR
#define Port_reset_LCD_OUT P5OUT
#define BIT_LCD_reset BIT2
#define LCD_ADDRESS 0x3E //7 bits //LCD 2x16 de las practicas de MiSE
#define ALIM_3V //Comentar esta linea si alim 5V
#ifdef ALIM_3V
	#define BOOSTER 0x54 //booster On
	#define FOLLOWER 0x6F //follower ON, y con amplificacion maxima para alim 3.3V
#else
	#define BOOSTER 0x50 //booster Off
	#define FOLLOWER 0x6D //follower ON, con menos amplificacion para alim 5V
#endif

//Recursos Maqueen+: (i2c)
#define Maqueen_ADDRESS 0x10 //@Maqueen Plus
#define CMD_MOTOR 0x00 //comando "motores" del Maqueen
#define CMD_RGB 0x0B   //comando "LEDs" del Maqueen
#define CMD_SERVO1 0x14 //comando "Servo1 (S1)" del Maqueen
#define CMD_SERVO2 0x15 //comando "Servo1 (S2)" del Maqueen
#define CMD_SERVO3 0x16 //comando "Servo1 (S3)" del Maqueen
#define CMD_LINE   0x1D //comando "Sensores del seguidor de linea" del Maqueen
#define CMD_COMP_LEFT 0x08
#define CMD_COMP_RIGHT 0x09



#define COLOR_RGB_F 2 //Front = verde
#define COLOR_RGB_R 1 //Rear = rojo

#define LED_MAQUEEN_STRING      "ML" //Leds del Maqueen
#define MOTOR_MAQUEEN_STRING    "MM" //Motores del Maqueen
#define SERVO_STRING            "MS" //Servos del Maqueen
//#define mas cosas??

//Intrucciones "Bioloid":
#define INSTR_PING 0x01
#define INSTR_READ 0x02
#define INSTR_WRITE 0x03
#define INSTR_REG_WR 0x04
#define INSTR_ACTION 0x05
#define INSTR_RESET 0x06
#define INSTR_SYNC_WRT 0x83

//Recursos para los modulos Bioloid:
#define Led 0x19
#define Motor 0x20
#define ADC_value 0x2D
#define Dist_value 0x28 //0x28 a 0x2B: float, 4 bytes
#define IR_Left 0x1A //Sensor izq.
#define IR_Center 0x1B //Sensor central
#define IR_Right 0x1C //Sensor der.
#define inst_write 0x03
#define ID_Left 0x01
#define ID_Right 0x02
#define ID_Broadcast 0xFE
#define ID_Motor_Test ID_Left

//Recursos para los ultrasonidos:
#ifdef PLACA_TEST //P0 = P6.0 (Timer), P8 = P6.3 (Trigger)
	#define P_TRIGGER_OUT P6OUT
	#define P_TRIGGER_DIR P6DIR
#else //P2 = P6.0 (Timer), P8 = P4.3 (Trigger)
	#define P_TRIGGER_OUT P4OUT
	#define P_TRIGGER_DIR P4DIR
#endif

#endif /* RECURSOS_H_ */
