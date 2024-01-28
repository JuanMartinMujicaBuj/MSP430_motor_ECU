/***************************************************************************//**
  @file     joystick_2.h
  @brief    Joystick services. Uses adc_2.
  @authors  Capparelli & Mujica Buj
 ******************************************************************************/

#ifndef JOYSTICK_2_H_
#define JOYSTICK_2_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include "common.h"

/*******************************************************************************
 * PUBLIC FUNCTION PROTOTYPES
 ******************************************************************************/
// Inicializa un pin para el joystick (entrada analogica). En este, se inicializa
// un ADC, con su función de callback. También calibra el 0 del joystick (el 511
// del ADC). Para eso, necesita ser llamada luego de enable_interrupts().
// pin: para analog, se usa siempre el puerto 1 (P1.x).
void joystickInit(uint8_t pin);

//Leer el ultimo valor guardado del joystick. En el medio hace una conversion del
// 0 a 1023 del ADC, al -10 a +10 del joystick.
int16_t joystickRead(void);

/******************************************************************************/
#endif /* JOYSTICK_2_H_ */
