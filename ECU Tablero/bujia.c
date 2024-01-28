/***************************************************************************//**
  @file     bujia.c
  @brief    Spark plug (bujía) services.
  @authors  Capparelli, Mujica Buj, Olivera, Torres, Zannier
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include "bujia.h"
#include "gpio.h"

/*******************************************************************************
 * STATIC VARIABLES AND CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/
static uint8_t bujiaEncendida;
static uint8_t pin_bujia;

/*******************************************************************************
* GLOBAL FUNCTION DEFINITIONS
*******************************************************************************/

void bujiaInit(uint8_t pinBujia, void (*rtiTimer)(rti_callback_t, unsigned int))
{
    gpioMode(pinBujia,OUTPUT);
    pin_bujia = pinBujia;

    if(*rtiTimer)
    {
        (*rtiTimer)(bujiaUpdate,1);
    }

}

/******************************************************************************/

void bujiaUpdate(void)
{
    gpioWrite(pin_bujia,bujiaEncendida);
}

/******************************************************************************/

uint8_t readBujiaVal(void)
{
    return bujiaEncendida;
}

/******************************************************************************/

void writeBujiaVal(uint8_t bujiaStatus)
{
    bujiaEncendida = bujiaStatus;
}

/******************************************************************************/

void writeBujia(uint8_t bujiaStatus)
{
    writeBujiaVal(bujiaStatus);
    bujiaUpdate();
}

/******************************************************************************/
