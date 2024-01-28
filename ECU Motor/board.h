/***************************************************************************//**
  @file     board.h
  @brief    Board management.
  @authors  Magliola, Mujica Buj, Olivera, Torres, Zannier Diaz
 ******************************************************************************/

#ifndef BOARD_H_
#define BOARD_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include "gpio.h"

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/
// Boton de la placa
#define BUTTON_MSK PORTNUM2PIN(1,3) //P1.3

// Led rojo de la placa
#define RED_LED_MSK PORTNUM2PIN(1,0) //P1.0

// Led verde de la placa
#define GREEN_LED_MSK PORTNUM2PIN(1,6) //P1.6

// Pines de UART (capaz ni se usen estos defines)
#define UART_RX PORTNUM2PIN(1,1) //P1.1
#define UART_TX PORTNUM2PIN(1,2) //P1.2

//Pines de SPI-CAN (de caracter informativo)
#define CAN_INT  PORTNUM2PIN(1,4) //P1.4
#define CAN_CS   PORTNUM2PIN(2,5) //P2.5
#define SPI_SCK  PORTNUM2PIN(1,5) //P1.5
#define SPI_MISO PORTNUM2PIN(1,6) //P1.6
#define SPI_MOSI PORTNUM2PIN(1,7) //P1.7

// Puerto analogico - joystick (se elige este pin entre varios posibles)
#define ADC_MSK PORTNUM2PIN(1,3) //P1.3

//Pin de pwm para servo: P2.1
//Pin de pwm para motor dc: P2.1
//Pin de input capture con opto: P1.2


/******************************************************************************/

#endif /* BOARD_H_ */
