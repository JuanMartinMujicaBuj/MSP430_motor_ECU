/***************************************************************************//**
  @file     servo.c
  @brief    Servo services.
  @authors  Capparelli, Mujica Buj, Olivera, Torres, Zannier Diaz
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include "optoacoplador.h"
#include "inputCapture.h"

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/
#define ICAP2OPTOVEL 15000 //Cte de transformacion de tiempo registrado por el IC a velocidad de motor.

/*******************************************************************************
 * STATIC VARIABLES AND CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/
static uint16_t  velOpto;
static uint8_t timer_ID;

/*******************************************************************************
* GLOBAL FUNCTION DEFINITIONS
*******************************************************************************/
//Inicializacion del Driver
void optoInit(uint8_t pin, uint8_t timer_num, void (*callback_capture)(void), void (*rtiTimer)(rti_callback_t, unsigned int))
{
    inputCaptureInit(pin, timer_num, CAPTURE_ON_RISING_EDGE_IC, NULL, *callback_capture);

    timer_ID = timer_num;

    if(*rtiTimer)
    {
        (*rtiTimer)(optoUpdate,1);
    }

}

/******************************************************************************/
//Leer el valor de velocidad medido de la señal del optoacoplador y guardarlo en un registro interno para uso posterior. Util para interrupciones periodicas.
void optoUpdate(void)
{
    uint16_t ic_read;
    ic_read = inputCaptureRead();
    if(!ic_read)
    {
        velOpto=0;
    }
    else
    {
        velOpto = ICAP2OPTOVEL/ic_read;
    }
}

/******************************************************************************/
//Leer el valor del registro de velocidad. Util para uso de interrupciones periodicas.
uint16_t readOptoVal(void)
{
return velOpto;
}

/******************************************************************************/
//Modifica el valor del registro de velocidad
void writeOptoVal(uint16_t newVelOpto)
{
velOpto = newVelOpto;
}

/******************************************************************************/
//Leer el valor del Optoacoplador, obtiene la velocidad medida y devuelve el valor
uint16_t readOpto(void)
{
    optoUpdate();
    return velOpto;
}

/******************************************************************************/
