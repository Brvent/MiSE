/*
 * uart.h
 *
 *  Created on: 22 ene. 2021
 *      Author: C. Serre, UB
 */

#ifndef UART_H_
#define UART_H_

#define RX_BUFFER_SIZE 512 // buffer UART. Ojo: puede superar los 255 caracteres

//Estructura de datos de respuesta a una recepci�n.
typedef struct RxReturn
{
    uint8_t StatusPacket[RX_BUFFER_SIZE]; //Para almacenar la trama recibida
    uint8_t time_out;   //Indica si ha habido un problema de timeout durante la recepcion
    uint16_t num_bytes;//El numero de bytes recibidos. Ojo: puede superar los 255 caracteres => tipo de 16 bits
    //Se puede ampliar con mas campos si se considera oportuno.
    //...
}RxReturn;

/*
RxPacket() read data from UART buffer (received from Wifi module).
RxPacket() need a timeout parameter in x100us, in order to abort if no response.
RxPacket() return struct containing Received Packet.
 */
RxReturn RxPacket(uint32_t time_out); //timeout in x100us

/*
TxPacket() send data to Wifi module (UART).
TxPacket() needs 2 parameters:
     Length of parameters (number of characters to be sent)
     Pointer to parameters to send.
     Returns number of characters actually sent
 */
uint8_t TxPacket(uint8_t bParameterLength, const uint8_t *Parameters);

void init_uart_wifi(void); //Configuracion de la UART


#endif /* UART_H_ */
