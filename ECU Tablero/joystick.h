/***************************************************************************//**
  @file     joystick_2.h
  @brief    Joystick services. Uses adc_2.
  @authors  Capparelli, Mujica Buj, Olivera, Torres, Zannier
 ******************************************************************************/

#ifndef JOYSTICK_H_INCLUDED
#define JOYSTICK_H_INCLUDED

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include "common.h"
#include "rti.h"


/*******************************************************************************
 * PUBLIC FUNCTION PROTOTYPES
 ******************************************************************************/
// Inicializa un pin para el joystick (entrada analogica). En este, se inicializa
// un ADC, con su función de callback. También calibra el 0 del joystick (el 511
// del ADC). Para eso, necesita ser llamada luego de enable_interrupts().
// pin: para analog, se usa siempre el puerto 1 (P1.x).
void joystickInit(uint8_t pinJoystick, void (*rtiTimer)(rti_callback_t, unsigned int));

//Modifica el valor registrado del joystick
void joystickUpdate(void);

//Devuelve el registro del valor del joystick
int16_t readJoystickVal(void);

//Sobreescribe el valor registrado del valor del joystick
void writeJoystickVal(int16_t newJoystickPos);

//Modifica el valor registrado del joystick y lo devuelve
int16_t readJoystick(void);

//Leer el ultimo valor guardado del joystick, de -512 a 511
int16_t joystickRead(void);

/******************************************************************************/

#endif // JOYSTICK_H_INCLUDED
