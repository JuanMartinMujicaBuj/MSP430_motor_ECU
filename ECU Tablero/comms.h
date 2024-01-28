/***************************************************************************//**
  @file     comms.h
  @brief    Communication through UART and SPI-CAN.
  @authors  Mujica Buj
 ******************************************************************************/

#ifndef COMMS_H_
#define COMMS_H_
/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include "common.h"

/*******************************************************************************
 * PUBLIC FUNCTION PROTOTYPES
 ******************************************************************************/
//inicializar UART y SPI-CAN
//Uart necesita saber el largo de sus mensajes recibidos, para saber cuándo
//terminó de recibir un mensaje.
//commsInit(0) permite inicializar la comunicación CAN pero no la UART
void commsInit(uint8_t uart_msg_length);

//Enviar mensaje por uart
void UARTSend(uint8_t string[]);

//Enviar mensaje por CAN
void CANSend(uint8_t largo, uint8_t bytes[]);

//Recibir mensaje por uart
void UARTReceive(uint8_t *p2char);

//Recibir mensaje por CAN
void CANReceive(uint8_t largo, uint8_t *p2char);

//ver si hay mensajes nuevos de uart
uint8_t UARTStatus(void);

//ver si hay mensajes nuevos de CAN
uint8_t CANStatus(void);

//Armar mensajes de CAN con variables de 1 byte (int8_t, uint8_t).
//Guardo la variable [var8] en la posicion [pos] del array [can_to_send].
void CANBuildMsg8(uint8_t can_to_send[], uint8_t var8, uint8_t pos);

//Armar mensajes de CAN con variables de 2 byte (int16_t, uint16_t)
void CANBuildMsg16(uint8_t can_to_send[], uint16_t var16, uint8_t pos);

//Armar mensajes de CAN con variables de 4 bytes (int32_t, uint32_t)
void CANBuildMsg32(uint8_t can_to_send[], uint32_t var32, uint8_t pos);

//Armar mensajes de CAN con variables de 64 bytes (int64_t, uint64_t)
void CANBuildMsg64(uint8_t can_to_send[], uint64_t var64, uint8_t pos);

//Extraer variable de 1 byte de largo de un mensaje de CAN
uint8_t CANUnbuildMsg8(uint8_t can_received[], uint8_t pos);

//Extraer variable de 2 bytes de largo de un mensaje de CAN
uint16_t CANUnbuildMsg16(uint8_t can_received[], uint8_t pos);

//Extraer variable de 4 bytes de largo de un mensaje de CAN
uint32_t CANUnbuildMsg32(uint8_t can_received[], uint8_t pos);

//Extraer variable de 8 bytes de largo de un mensaje de CAN
uint64_t CANUnbuildMsg64(uint8_t can_received[], uint8_t pos);

//UART usa USCI_A0 y funciona por interrupts dedicadas para cargar los bits en el
//TX buffer, y para recibir en el RX buffer.
//SPI usa USCI_B0 y es bloqueante pero funciona a un mayor baud rate.
//CAN es implementado por las placas mcp2515+tja1050, a las que se les cargan
//instrucciones por SPI.

//CAN envia CAN frames, es decir, vectores de hasta 8 bytes. Esos bytes se
//pueden interpretar como (u)int8's, (u)int16's, etc, o una mezcla.
//Las funciones de armado de mensajes de CAN y extraccion de variables usan la
//codificacion: byte mas significativo primero (big endian).

//UART envia chars y strings. Si se quiere enviar una variable, se la debe
//convertir en un string. Por la gran variedad de formas que hay para hacer
//esto, el driver no incluye funciones de num2str.
/******************************************************************************/
#endif /* COMMS_H_ */
