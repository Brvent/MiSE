/*
 * AT.c
 *
 *  Created on: 01/2021
 *      Author: C. Serre, UB
 */

#include <msp430.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>


#include "AT.h"
#include LIBUART_AT //#include "uart_wifi.h"

uint8_t SSID_actual[32]; //El SSID del AP al que estamos conectado.

//Mensajes de comunicacion para uart<->wifi:
const uint8_t test[] = {"AT\r\n"}; //El comando mas basico para comprobar si el modulo responde. Deber�a contestar "OK"
const uint8_t quien_Soy[] = {"AT+CWSAP_CUR?\r\n"}; //Para pedir propio SSID cuando somos AP
const uint8_t clave[] = {"AT+CWSAP_PWD?\r\n"}; //Para pedir la contrasenya del AP (este, me lo he inventado yo...)
const uint8_t local_IP[] = {"AT+CIFSR\r\n"}; //Pedir propia IP (= la del AP)

const uint8_t lista_clientes[] = {"AT+CWLIF\r\n"}; //Pide la lista de clientes conectados a nosotros cuando somos AP
const uint8_t estado_conexiones[] = {"AT+CIPSTATUS\r\n"}; //Pedir info de la(s) conexion(es) con cliente(s)
const uint8_t version[] = {"AT+GMR\r\n"}; //Para comprobar la version de firmware del modulo

const uint8_t n_num_caract = 32; //numnero max de caracteres a enviar.
const uint8_t num_caract[] = {"32"}; //forma ASCII del numnero max de caracteres a enviar.
//const uint8_t retorno[] = {"\r\n"}; //0x0d 0x0A
//* No hace falta añadir a las cadenas el caracter nulo "\0" de terminacion,
//* el compilador ya lo hace.

const uint8_t Respuesta_OK [] = "AT OK";

/*
 * Un comando de test "AT".
 * La respuesta deberia ser "OK".
 */
uint8_t comando_AT(){
    struct RxATReturn respuesta;
    TxAT(sizeof(test)-1,test);
    //llamada a RxAT() para leer la respuesta:
    respuesta = RxAT(2000);
    if(respuesta.num_bytes) //hay una respuesta
        //supondremos que si hay respuesta, tiene que ser "OK"
        return 1; //exito
    return 0;
}

int8_t getConState(){
	struct RxATReturn respuesta;
    uint8_t *ptr, estado;
	TxAT(sizeof(estado_conexiones)-1, estado_conexiones);
	respuesta = RxAT(100);
    if(respuesta.num_bytes == 0)
        return ERR_NO_CONN;
    if(ptr = strstr(respuesta.StatusPacket, CONN_STATE_STRING)){
        estado = ptr[strlen(CONN_STATE_STRING)];
        return estado;
    }
    return 0;
}


//Pedir propia IP, sea cual sea su papel (AP o STA)
int8_t get_IP(uint8_t *AP_IP){
    struct RxATReturn respuesta;
    TxAT(sizeof(local_IP)-1, local_IP);
    respuesta = RxAT(100);
    if(respuesta.num_bytes == 0)
        return ERR_NO_IP;
    respuesta.StatusPacket[respuesta.num_bytes] = 0; // caracter nulo de fin de cadena
    // sprintf(ssid, "%s", respuesta.StatusPacket);
    memcpy(AP_IP, respuesta.StatusPacket, respuesta.num_bytes);
    return 0;
}

/*
 * Para preguntar pedir propio SSID
 */
int8_t get_AP_SSID( uint8_t *ssid){
    struct RxATReturn respuesta;
    respuesta.num_bytes = 0;
    TxAT(sizeof(quien_Soy)-1,quien_Soy);
    respuesta = RxAT(100);
    if(respuesta.num_bytes == 0)
        return ERR_NO_SSID;
    respuesta.StatusPacket[respuesta.num_bytes] = 0; // caracter nulo de fin de cadena
    // sprintf(ssid, "%s", respuesta.StatusPacket);
    memcpy(ssid, respuesta.StatusPacket, respuesta.num_bytes);
    return 0; 
}

/*
 * Para preguntar pedir propio password
 */

int8_t get_AP_PWD(uint8_t *pwd){
    struct RxATReturn respuesta;
    respuesta.num_bytes = 0;
    TxAT(sizeof(clave)-1,clave);
    respuesta = RxAT(100);
    if(respuesta.num_bytes == 0)
        return ERR_NO_SSID;
    respuesta.StatusPacket[respuesta.num_bytes] = 0; // caracter nulo de fin de cadena
    // sprintf(ssid, "%s", respuesta.StatusPacket);
    memcpy(pwd, respuesta.StatusPacket, respuesta.num_bytes);
    return 0;
}

struct RxATReturn enviar_wifi(uint8_t *trama, uint8_t medida){
    struct RxATReturn respuesta;
    TxAT(medida, trama);
    respuesta = RxAT(500); //50ms son suficientes?
    return respuesta;
}

//Comprobar posibles mensaje imprevistos que vengan por la wifi
//Se tendr� que llamar periodicamente desde el main()
struct RxATReturn recibir_wifi(){
    struct RxATReturn respuesta;
    respuesta.num_bytes = 0;
    respuesta = RxAT(500); //50ms: timeout relativamente largo, para maximizar el tiempo de escucha
    return respuesta;
}

