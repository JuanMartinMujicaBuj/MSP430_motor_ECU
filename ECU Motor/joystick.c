/***************************************************************************//**
  @file     joystick_2.c
  @brief    Joystick services. Uses adc_2.
  @authors  Capparelli, Mujica Buj, Olivera, Torres, Zannier
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include "joystick.h"
#include "adc_2.h"

/*******************************************************************************
 * STATIC VARIABLES AND CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

static int16_t zeroJoystick;
static int16_t joystickPos;

/*******************************************************************************
* GLOBAL FUNCTION DEFINITIONS
*******************************************************************************/
//Inicializacion del Driver
void joystickInit(uint8_t pinJoystick, void (*rtiTimer)(rti_callback_t, unsigned int))
{
      adcInit(pinJoystick); //Inicializa el adc
      zeroJoystick = adcRead(); //guarda el cero del joystick
      if(*rtiTimer)
      {
          (*rtiTimer)(joystickUpdate,1);
      }
}

/******************************************************************************/

//Leer nuevo valor de posicion del joystick del ADC
void joystickUpdate(void)
{
    joystickPos = (int16_t)(adcRead()-zeroJoystick);
}

/******************************************************************************/

// Leer el registro de la posicion del joystick
int16_t readJoystickVal(void)
{
    return joystickPos;
}

/******************************************************************************/

//Modificar el registro de la posicion del joystick
void writeJoystickVal(int16_t newJoystickPos)
{
    joystickPos = newJoystickPos;
}

/******************************************************************************/

//Modificar el registro de la posicion del joystick y devolverlo
int16_t readJoystick(void)
{
    joystickUpdate();
    return readJoystickVal();
}

/******************************************************************************/
