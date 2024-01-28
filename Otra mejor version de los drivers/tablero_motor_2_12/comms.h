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

//UART usa USCI_A0 y funciona por interrupts dedicadas para cargar los bits en el
//TX buffer, y para recibir en el RX buffer.
//SPI usa USCI_B0 y es bloqueante pero funciona a un mayor baud rate.
//CAN es implementado por las placas mcp2515+tja1050, a las que se les cargan
//instrucciones por SPI.
/******************************************************************************/
#endif /* COMMS_H_ */
