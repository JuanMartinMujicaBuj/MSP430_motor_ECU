/***************************************************************************//**
  @file     dc_motor.h
  @brief    dc motor services: pwm.
  @authors  Mujica Buj
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include "dc_motor.h"
#include "pwm.h"

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/
#define PERIODO 2000 //1ms de periodo para el motor dc. Se elige semi arbitrariamente.

/*******************************************************************************
 * STATIC VARIABLES AND CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/
static uint16_t t_on; //t_on del pwm que controla el dc motor.
static uint8_t timer_num_copy;

/*******************************************************************************
* GLOBAL FUNCTION DEFINITIONS
*******************************************************************************/
//Inicializar timer, pwm y dc_motor.
void dcMotorInit(uint8_t timer_num, uint16_t velocidad_x10, uint8_t pin)
{

    //t_on = velocidad_x10*PERIODO/(DC_VEL_MAX*10); //tiempo en HIGH de los 20ms del ciclo, que me da la velocidad deseada.
    t_on = (uint16_t)( ((uint32_t)(velocidad_x10) *(PERIODO/10))/DC_VEL_MAX );

    if(t_on<PERIODO/200) //limito el duty cycle a entre 0.5% y 99.5% (para 20ms, un ccr1 entre 100 y 19900, con ccr0=20000)
    {
        t_on=PERIODO/200;
    }
    else if(t_on>(PERIODO/200)*199)
    {
        t_on=(PERIODO/200)*199;
    }

    pwmInit(timer_num, HIGH_LOW_PWM, PERIODO, t_on, pin);

    timer_num_copy = timer_num;
}

/******************************************************************************/
//Cambiar velocidad.
void dcMotorWrite(uint16_t velocidad_x10)
{
    t_on = (uint16_t)( (uint32_t)(velocidad_x10) *(PERIODO/10)/DC_VEL_MAX ); //recalculo el tiempo on que me da la velocidad deseada.

    if(t_on<PERIODO/200) //limito el duty cycle a entre 0.5% y 99.5% (para 20ms, un ccr1 entre 100 y 19900, con ccr0=20000)
    {
        t_on=PERIODO/200;
    }
    else if(t_on>(PERIODO/200)*199)
    {
        t_on=(PERIODO/200)*199;
    }

    pwmWrite(timer_num_copy, PERIODO, t_on);
}

/******************************************************************************/
//Frenar/apagar el motor.
void dcMotorStop(void)
{
    pwmStop(timer_num_copy);
    //ver si este tambien APAGA el pin de salida, o si eso lo hace el hwTimer.
}

/******************************************************************************/
