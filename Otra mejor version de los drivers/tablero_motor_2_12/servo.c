/***************************************************************************//**
  @file     servo.c
  @brief    Servo services.
  @authors  Capparelli & Mujica Buj
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include "servo.h"
#include "pwm.h"

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/
#define PERIODO 20000 //20ms de periodo para el servo

/*******************************************************************************
 * STATIC VARIABLES AND CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/
static uint16_t t_on; //t_on del pwm que controla el servo
static uint8_t timer_num_copy;

/*******************************************************************************
* GLOBAL FUNCTION DEFINITIONS
*******************************************************************************/
//Inicializar timer, pwm y servo.
// El servo trabaja con un periodo de 20ms, con un t_on de 0.5ms para -90° a
// 2.5ms para +90° (micro servo 9g)
void servoInit(uint8_t timer_num, int8_t angulo, uint8_t pin)
{

    t_on=(uint16_t)((uint32_t)(angulo+90)*2000/180)+500; //tiempo en HIGH de los 20ms del ciclo, que me da el angulo deseado

    pwmInit(timer_num, HIGH_LOW_PWM, PERIODO, t_on, pin);

    timer_num_copy = timer_num;
}

/******************************************************************************/
//Cambiar angulo.
void servoWrite(int8_t angulo)
{
    t_on=(uint16_t)((uint32_t)(angulo+90)*2000/180)+500; //recalculo el tiempo on que me da el angulo deseado
    pwmWrite(timer_num_copy, PERIODO, t_on);
}

/******************************************************************************/
//Frenar/apagar el servo.
void servoStop(void)
{
    pwmStop(timer_num_copy);
    //ver si este tambien APAGA el pin de salida, o si eso lo hace el hwTimer
}

/******************************************************************************/
