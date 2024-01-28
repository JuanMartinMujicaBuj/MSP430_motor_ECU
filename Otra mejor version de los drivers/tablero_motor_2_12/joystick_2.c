/***************************************************************************//**
  @file     joystick_2.c
  @brief    Joystick services. Uses adc_2.
  @authors  Capparelli & Mujica Buj
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include "joystick_2.h"
#include "adc_2.h"

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/
#define adc2joystick   1/50 //en teoría es 2/102.4, para tener una salida de -10
                             //a +10. En la práctica, para llegar al +-10, dividimos
                             //por un nro un poco mas chico.

/*******************************************************************************
 * STATIC VARIABLES AND CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/
uint16_t zeroJoystick;

/*******************************************************************************
* GLOBAL FUNCTION DEFINITIONS
*******************************************************************************/
void joystickInit(uint8_t pin)
{
    adcInit(pin); //Inicializa el adc
    zeroJoystick = adcRead(); //guarda el cero del joystick
}

/******************************************************************************/

int16_t joystickRead(void)
{
    return (int16_t)(adcRead()-zeroJoystick)*adc2joystick;
}

/******************************************************************************/
