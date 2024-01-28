/***************************************************************************//**
  @file     inputCapture.c
  @brief    input capture services using a timer.
  @authors  Mujica Buj, Olivera
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include "inputCapture.h"
#include "hw_timer.h"

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/
#define TICKS2MILLIS 1/1000 //para el clock del sistema yendo a 1MHz.
#define TICKS_OVERFLOW 50000 //se eligen 50ms, numero mas redondo que 60.535

/*******************************************************************************
 * ENUMERATIONS, STRUCTURES AND TYPEDEFS
 ******************************************************************************/
typedef void (*callback_t)(void);

/*******************************************************************************
 * STATIC VARIABLES AND CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/
static uint16_t tBetweenCaptures; //tiempo entre los ultimos dos flancos detectados, en ms
static uint8_t nOverflows; //cantidad de overflows que hubieron entre dos captures
static uint16_t oldCaptureTime; //valor del free running counter cuando ocurre el anteultimo capture
static uint16_t newCaptureTime; //valor del free running counter cuando ocurre el ultimo capture
static uint8_t timer_num; //valor del timer usado.
static callback_t capture;
static callback_t overflow;


/*******************************************************************************
* PRIVATE FUNCTION PROTOTYPES
*******************************************************************************/
void rti_Overflow_CCR0(void);
void rti_Capture_CCR1(void);

/*******************************************************************************
* GLOBAL FUNCTION DEFINITIONS
*******************************************************************************/
//Inicializar un timer en modo input capture. Para este driver, usa count up
//mode y pines: los que trabajan con CCR1 o CCR2.
//Puede detectar flancos ascendentes, descendentes o ambos.
void inputCaptureInit(uint8_t pin, uint8_t timer_num_in, uint8_t capture_mode, void (*callback_overflow)(void), void (*callback_capture)(void))
{
    timer_num = timer_num_in;

    hwTimerInit(timer_num);
    hwSetTimer(timer_num, INPUT_CAPTURE_HWT, UP_MODE_HWT, capture_mode, 0, pin, TICKS_OVERFLOW, 0, *rti_Overflow_CCR0, *rti_Capture_CCR1);
    capture =  callback_capture;
    overflow =  callback_overflow;
}

/******************************************************************************/
//Leer tiempo entre dos captures, en milisegundos
uint16_t inputCaptureRead(void)
{
    return tBetweenCaptures;
}

/*******************************************************************************
* PRIVATE FUNCTION PROTOTYPES
*******************************************************************************/
void rti_Overflow_CCR0(void)
{
    if(*overflow)
    {
       (*overflow)();
    }
    nOverflows += 1;
}

/******************************************************************************/
void rti_Capture_CCR1(void)
{
    newCaptureTime = hwTimerGetCCR1(timer_num);
    if(*capture)
    {
        (*capture)();
    }


    tBetweenCaptures = nOverflows * (TICKS_OVERFLOW * TICKS2MILLIS) + newCaptureTime * TICKS2MILLIS - oldCaptureTime * TICKS2MILLIS;

    nOverflows = 0;
    oldCaptureTime = newCaptureTime;
}
