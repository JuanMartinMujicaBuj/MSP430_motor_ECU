/***************************************************************************//**
  @file     uart.h
  @brief    UART serial communication services.
  @authors  Jacoby, Mujica Buj
 ******************************************************************************/

#ifndef UART_H_
#define UART_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include "common.h"

/*******************************************************************************
 * PUBLIC FUNCTION PROTOTYPES
 ******************************************************************************/
//Inicializa transmision y recepcion de datos por UART, con el modulo USCI_A0.
//Habilita interrupts dedicadas de recepcion de datos (RXIE), que se iran
//acumulando en el receive_buffer que tiene el driver.
//Setea cuántos chars recibidos significan que recibí un mensaje.
void uartInit(uint8_t n_char);

//Escribe un char en el transmit_buffer que tiene el driver.
void uartWriteChar(uint8_t c);

//Escribe un string en el transmit_buffer que tiene el driver.
void uartWriteString(uint8_t string[]);

//Habilita interrupts dedicadas de transmision de datos (TXIE) para enviar todos
//los chars acumulados en el transmit_buffer (y los borra del transmit_buffer).
void uartSend(void);

//Lee el ultimo char que llego y se guardo en el receive_buffer.
uint8_t uartReadChar(void);

//Lee todos los chars que se acumularon en el receive_buffer (y los borra del receive_buffer).
void uartRead(uint8_t *p2char);

//Avisa si recibí un msj: devuelve 1 si acumulé n_char en el receive buffer; si no, 0.
uint8_t uartStatus(void);

//Rutinas para las interrupciones dedicadas
void uart_transmit_rti(void);
void uart_receive_rti(void);

/******************************************************************************/
#endif /* UART_H_ */
