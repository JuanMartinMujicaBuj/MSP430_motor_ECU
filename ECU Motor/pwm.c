/***************************************************************************//**
  @file     pwm.c
  @brief    PWM services.
  @authors  Capparelli & Mujica Buj
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include "pwm.h"
#include "hw_timer.h"

/*******************************************************************************
 * STATIC VARIABLES AND CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/
static uint16_t t_periodo_copy[2]; //2 valores: para Timer0_A y Timer1_A
static uint16_t t_on_copy[2];

static uint8_t t_periodo_flag[2];
static uint8_t t_on_flag[2];

/*******************************************************************************
 * PRIVATE FUNCTION PROTOTYPES
 ******************************************************************************/
void update_t_periodo0(void); //para Timer0_A, para actualizar CCR0-->t_periodo
void update_t_on0(void); //para actualizar CCR1-->t_on
void update_t_periodo1(void); //para Timer1_A
void update_t_on1(void);

/*******************************************************************************
* GLOBAL FUNCTION DEFINITIONS
*******************************************************************************/
//Inicializar timer. Inicializar pwm: elegir timer (A0 o A1), modo
//(HIGH_LOW, LOW_HIGH, CENTERED), tiempo total del periodo y tiempo encendido.
void pwmInit(uint8_t timer_num, uint8_t modo, uint16_t t_periodo, uint16_t t_on, uint8_t pin)
{
    uint8_t count_mode;
    uint8_t out_mode;

    //elegir si uso timer_A0 o timer_A1
    hwTimerInit(timer_num);

    switch(modo)
    {
    case HIGH_LOW_PWM:
        count_mode = UP_MODE_HWT;
        out_mode = RESET_SET_MODE_HWT;
        break;
    case LOW_HIGH_PWM:
        //codear
        break;
    case CENTERED_PWM:
        //codear
        break;
    }

    //Elijo modo y seteo ccr0, ccr1 y funciones de callback del hw_timer.
    //Esas funciones de callback son usadas para corregir ccr0, ccr1, si hubo un cambio.
    if(!timer_num) //Timer0_A
    {
        hwSetTimer(0, OUTPUT_COMPARE_HWT, count_mode, NO_CAPTURE, out_mode, pin, t_periodo, t_on, *update_t_periodo0, *update_t_on0);
    }
    else //Timer1_A
    {
        hwSetTimer(1, OUTPUT_COMPARE_HWT, count_mode, NO_CAPTURE, out_mode, pin, t_periodo, t_on, *update_t_periodo1, *update_t_on1);
    }

    t_periodo_copy[timer_num]=t_periodo;
    t_on_copy[timer_num]=t_on;
}

/******************************************************************************/
//Cambiar tiempo total del periodo y tiempo encendido.
void pwmWrite(uint8_t timer_num, uint16_t t_periodo, uint16_t t_on)
{
    if(t_periodo != t_periodo_copy[timer_num])
    {
        t_periodo_flag[timer_num]=1;
    }
    if(t_on != t_on_copy[timer_num])
    {
        t_on_flag[timer_num]=1;
    }

    t_periodo_copy[timer_num]=t_periodo;
    t_on_copy[timer_num]=t_on;
}

/******************************************************************************/
//Frenar el pwm
void pwmStop(uint8_t timer_num)
{
    hwStopTimer(timer_num);
    //de ser deseado, se debe poner en LOW el pin de salida por fuera de esta función.
}

/*******************************************************************************
 * PRIVATE FUNCTION DEFINITIONS
 ******************************************************************************/
void update_t_periodo0(void)
{
    if(t_periodo_flag[0])
    {
        hwTimerSetCCR0(t_periodo_copy[0], 0);
        t_periodo_flag[0]=0;
    }
}

/******************************************************************************/
void update_t_on0(void)
{
    if(t_on_flag[0])
    {
        hwTimerSetCCR1(t_on_copy[0], 0);
        t_on_flag[0]=0;
    }
}

/******************************************************************************/
void update_t_periodo1(void)
{
    if(t_periodo_flag[1])
    {
        hwTimerSetCCR0(t_periodo_copy[1], 1);
        t_periodo_flag[1]=0;
    }
}

/******************************************************************************/
void update_t_on1(void)
{
    if(t_on_flag[1])
    {
        hwTimerSetCCR1(t_on_copy[1], 1);
        t_on_flag[1]=0;
    }
}

/******************************************************************************/
