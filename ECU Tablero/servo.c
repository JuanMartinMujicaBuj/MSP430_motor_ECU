/***************************************************************************//**
  @file     servo.c
  @brief    Servo services.
  @authors  Capparelli, Mujica Buj, Olivera, Torres, Zannier
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
#define MIN_PWM 500  //minimo tiempo prendido del PWM. Correspondiente a -90°
#define MAX_PWM 2500  //maximo tiempo prendido del PWM. Correspondiente a +90°
#define PWM_RANGE 2000 //rango de tiempo prendido del PWM (maximo - minimo)

/*******************************************************************************
 * STATIC VARIABLES AND CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

static int8_t pos;
static uint16_t t_on;
static uint8_t timer_ID;

/*******************************************************************************
* GLOBAL FUNCTION DEFINITIONS
*******************************************************************************/

//Inicializacion del Driver
void servoInit(uint8_t pinServoPWM, uint8_t timer_num, void (*rtiTimer)(rti_callback_t, unsigned int))
{
    pwmInit(timer_num, HIGH_LOW_PWM, PERIODO, MIN_PWM, pinServoPWM);
    timer_ID = timer_num;
    if(*rtiTimer)
    {
        (*rtiTimer)(servoUpdate,1);
    }
}

/******************************************************************************/

//Modificar la señal PWM conectada al servo. Para usar con interrupciones periodicas en conjunto con writeServoPos
void servoUpdate(void)
{
    t_on=(uint16_t)((uint32_t)(pos+90)*PWM_RANGE/180)+MIN_PWM; //recalculo el tiempo on que me da la posicion angular deseada
    pwmWrite(timer_ID, PERIODO, t_on);
}

/******************************************************************************/

//Leer la posicion angular actual del servo
int8_t readServoPos(void)
{
    return pos;
}

/******************************************************************************/

//Modificar el registro de posicion del servo. Para usar con interrupciones periodicas en conjunto con servoUpdate
void writeServoPos(int8_t newPos)
{
    pos=newPos;
}

/******************************************************************************/

//Modifico el registro de posicion y la señal PWM en conjunto. Para usar dentro de codigo.
void servoWrite(int8_t newPos)
{
    writeServoPos(newPos);
    servoUpdate();
}

/******************************************************************************/

//Frenar/apagar el servo.
void servoStop(void)
{
    pwmStop(timer_ID);
    pos = 0;
}

/******************************************************************************/
