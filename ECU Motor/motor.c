/***************************************************************************//**
  @file     motor.h
  @brief    dc motor services: pwm.
  @authors  Capparelli, Mujica Buj, Olivera, Torres, Zannier
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include "motor.h"
#include "pwm.h"

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/
#define PERIODO 2000 //2ms de periodo para el motor dc.

/*******************************************************************************
 * STATIC VARIABLES AND CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

static uint16_t velMotor_x10;
static uint8_t timer_ID;
static uint16_t t_on;//t_on del pwm que controla el dc motor.

/*******************************************************************************
* GLOBAL FUNCTION DEFINITIONS
*******************************************************************************/

//Inicializacion del Driver
void motorInit(uint8_t pinMotorPWM, uint8_t timer_num, void (*rtiTimer)(rti_callback_t, unsigned int))
{
    pwmInit(timer_num, HIGH_LOW_PWM, PERIODO, 100, pinMotorPWM);
    timer_ID = timer_num;

    if(*rtiTimer)
    {
        (*rtiTimer)(motorUpdate,1);
    }
}

/******************************************************************************/

//Modificar la señal de PWM conectada al motor. Para uso con interrupciones periodicas en conjunto con writeVelMotor
void motorUpdate(void)
{
    t_on = (uint16_t)( (uint32_t)(velMotor_x10) *(PERIODO/10)/DC_VEL_MAX ); //recalculo el tiempo on que me da la velocidad deseada.

    if(t_on<PERIODO/200) //limito el duty cycle a entre 0.5% y 99.5%
    {
        t_on=PERIODO/200;
    }
    else if(t_on>(PERIODO/200)*199)
    {
        t_on=(PERIODO/200)*199;
    }

    pwmWrite(timer_ID, PERIODO, t_on);

}

/******************************************************************************/

//Leer la velocidad actual del motor
uint8_t readVelMotor(void)
{
    return velMotor_x10/10;
}

/******************************************************************************/

//Modificar el registro de velocidad. Para uso con interrupciones periodicas en conjunot con motorUpdate
void writeVelMotor(uint16_t newVelMotor)
{
    velMotor_x10 = newVelMotor*10;
}

/******************************************************************************/

//Modifica el registro de velocidad y la señal del pwm conectada al motor. Para uso dentro de codigo.
void writeMotor(uint16_t newVelMotor)
{
    writeVelMotor(newVelMotor);
    motorUpdate();
}

/******************************************************************************/

//Frenar el motor. Setear el registro de velocidad a 0.
void motorStop(void)
{
    pwmStop(timer_ID);
    velMotor_x10 = 0;
}

/******************************************************************************/
