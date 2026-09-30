/*
 * AT.h
 *
 *  Created on: 01/2021
 *      Author: c. Serre, UB
 */

#ifndef AT_H_
#define AT_H_

#include <stdint.h>

/*
 * Esta libreria necesita otra libreria con las funciones especificas de la UART (p. ej "uart_wifi.h"),
 * a programar/adaptar para cada micro, y que contenga las siguientes funciones:
 *
    RxReturn RxPacket(uint32_t time_out);   //El timeout deber�a ser un valor en el rango de los ms, en base a un timer de 100us.
                                            //Devuelve una struct con la respuesta recibida.

    uint8_t TxPacket(uint8_t bParameterLength, const uint8_t *Parameters);
                      //Los parametros son el numero de bytes a enviar y un puntero a los datos a enviar.
                      //Devuelve el numero de bytes realmente enviados. Es opcional, podr�a ser void.
 *
 * RxReturn debe ser una struct para almacenar la respuesta, y debe tener como m�nimo los siguientes campos
 * para el buen funcionamiento de la libreria AT:
      typedef struct RxReturn
        {
        uint8_t StatusPacket[RX_BUFFER_SIZE]; //Para almacenar la trama recibida
        uint8_t time_out;   //Indica si ha habido un problema de timeout durante la recepcion
        uint16_t num_bytes;//El numero de bytes recibidos. Ojo: puede superar los 255 caracteres => tipo de 16 bits
        //Se puede ampliar con mas campos si se considera oportuno.
        //...
    }RxReturn;
 *
 * Los nombres
     RxReturn
     RxPacket
     TxReturn
    "uart_wifi.h"
 * se pueden cambiar, pero se tendran que indicar a la libreria AT,  editando las siguientes lineas,
 * para asignar los nombres reales de vuestra(s) libreria(s) a los nombres que se espera la libreria AT:
 */
#define LIBUART_AT "uart_alumnos.h" //el nombre de la libreria real de la UART
#define RxAT RxPacket           //el nombre real de la funcion de recepcion por la UART
#define TxAT TxPacket           //el nombre real de la funcion de transmision por la UART
#define RxATReturn RxReturn     //el nombre real de la struct para las respuestas recibidas por la UART


/*
 *  A partir de aqui, lo que sigue NO se deberia alterar!
 */

//Recursos para el ESP-01:
#define CONNECTED           0x08 //Otro equipo se acaba de conectar a nuestro modulo
#define CONEXION_STRING "CONNECT"
#define DISCONNECTED        0x10 //Un equipo conectado se acaba de desconectar.
#define DISCONEXION_STRING "CLOSED"
#define CONN_STATE_STRING "CIP_STATUS:"

//Codigos de errores:
#define ERR_NO_AT -1 //El modulo no responde
#define ERR_NO_AP -2 //No se ha detetado el AP
#define ERR_NO_SSID -3 //No se ha podido leer el nombre del AP
#define ERR_NO_IP -4 //No se ha podido leer la IP del AP
#define ERR_NO_CONN -5 //No se ha podido leer el estado de las conexiones al AP
#define ERR_TEXT_OVERFLOW -6 //El texto recibido es mas largo que el buffer de recepcion
#define ERR_CMD -7 //El comando recibido no se reconoce

/************************************************************************
 * Lee la IP en una cadena adecuada.
 * cadena: la cadena imprimible a devolver
 * Devuelve 0 (exito) o mensage de error < 0
 */
int8_t get_IP(uint8_t *AP_IP);

/************************************************************************
 * Lee el SSID y password en sendas cadenas 
 * ssid: para almacenar el SSID como cadena imprimible a devolver
 * pwd: para almacenar el password como cadena imprimible a devolver
 * Puede retornar un valor negativo si hay error:
 * devuelve 0 (exito) o mensage de error < 0
 */
int8_t get_AP_SSID( uint8_t *ssid);
int8_t get_AP_PWD(uint8_t *pwd);

/************************************************************************
 * Un comando de test. Se mandan los caracteres ASCII
 *  'A' 'T' 0x0D 0x0A
 *  (equivalente a la cadena "AT\r\n")
 *  por la uart al modulo Wifi.
 * La respuesta deberia ser "OK".
 * Devuelve 1 si hay respuesta, 0 si el modulo no responde.
 */
uint8_t comando_AT();

/************************************************************************
 * Funcion para saber si tenemos algun cliente conectado.
 * (Solo devuelve el valor de una variable de la libreria, no se comunica con el modulo Wifi)
 * 0: no hay ningun cliente
 * 1: hay al menos 1 cliente
 */
int8_t getConState();

/*************************************************************************
 * Funcion para enviar una trama (o cualquier mensaje) por la Wifi
 * Los parametros son
       un puntero al buffer de datos a enviar (*mensaje)
       el numero de datos a enviar (medida)
 * Devuelve la respuesta en una struct
 */
struct RxATReturn enviar_wifi(uint8_t *mensaje, uint8_t medida);

/*
 * Funcion para comprobar posibles mensaje o tramas imprevistos (que no sean una respuesta
 * a la funcion enviar_wifi) que vengan por la wifi.
 * Se tendria que llamar periodicamente desde el main()
 * El mensaje recibido estara en la struct.
 * Si acaba con un timeout sin que haya llegado ningun mensaje, el numero de caracteres (num_bytes)
 * de la struc tendra valor 0.
 */
struct RxATReturn recibir_wifi();


#endif /* AT_H_ */
