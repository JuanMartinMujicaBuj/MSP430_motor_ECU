/***************************************************************************//**
  @file     rti.c
  @brief    Periodic interrupts and callback management.
  @author   Mujica Buj
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include "rti.h"
#include <msp430.h>

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/
#define MAX_CALLBACKS 10 //Nro maximo de callbacks (modificable)

/*******************************************************************************
 * STATIC VARIABLES AND CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/
static rti_callback_t rtiCallback[MAX_CALLBACKS]; //Arreglo de punteros a funciones de callback
static uint16_t callbackTime[MAX_CALLBACKS]; //Arreglo de frecuencias con las que llamar a cada callback
static uint16_t callbackCounter[MAX_CALLBACKS]; //Contador de llamados de la interrupcion periodica del timer
static uint8_t nroCallbacks=0; //Nro de callbacks guardados en el arreglo rtiCallback[]
static uint8_t i; //para recorrer los callbacks.
/*******************************************************************************
* GLOBAL FUNCTION DEFINITIONS
*******************************************************************************/
uint8_t rtiSubmitCallback (rti_callback_t fun, uint16_t fun_frequency)
{
    if ((fun != NULL) && nroCallbacks<=MAX_CALLBACKS)
    {
        rtiCallback[nroCallbacks] = fun; //Guarda puntero a funcion en el vector de callbacks
        callbackTime[nroCallbacks] = fun_frequency; //Guarda frecuencia con la que llamar a cada callback
        callbackCounter[nroCallbacks] = callbackTime[nroCallbacks]; //Cuenta desde callbackTime hasta 0
    }
    return nroCallbacks++; //devuelve un id del callback y suma 1 al nro de callbacks guardados
}

/******************************************************************************/
void rtiClearCallback (uint8_t callback_id)
{
    rtiCallback[callback_id] = NULL;
    callbackTime[callback_id] = UINT16_MAX;
    callbackCounter[callback_id] = UINT16_MAX;

    /* Lo de abajo no lo puedo hacer porque estoy cambiando el callback_id de otras funciones!
    for(i=callback_id; i<nroCallbacks-1 ;++i)
    {
        rtiCallback[i]=rtiCallback[i+1];         //si por ejemplo tengo hasta rtiCallback[15],
        callbackTime[i]=callbackTime[i+1];       //nroCallbacks=16, y quiero borrar el 14: en [14]
        callbackCounter[i]=callbackCounter[i+1]; //copio lo de [15], y en [15] copio NULL/0.
    }
    rtiCallback[nroCallbacks-1]=NULL;
    callbackTime[nroCallbacks-1]=0;
    callbackCounter[nroCallbacks-1]=0;

    nroCallbacks--; //resto 1 al nro de callbacks guardados
    */
}

/*******************************************************************************
 * PRIVATE FUNCTION DEFINITIONS
 ******************************************************************************/
void recorrerCallbacks(void)
{
    for(i=0;i<nroCallbacks;++i) //Para todos los callbacks guardados:
        {
            if (!callbackCounter[i]) //Si se cumple la frecuencia pedida de llamado de cada callback:
            {
                (*(rtiCallback[i]))(); //Llama por puntero a funcion al callback guardado
                callbackCounter[i] = callbackTime[i];
            }
            callbackCounter[i]--; // Incrementa el contador de llamados a la interrupcion periodica del timer
        }
}

/******************************************************************************/
/******************************************************************************/
//Segun que modulo se use para generar las interrupciones periodicas (WDT o TA1),
//Se llamara a un __interrupt o al otro.

#pragma vector = WDT_VECTOR             //Interval timer vector location
__interrupt void rti_isr_wdt(void)
{
    recorrerCallbacks();
}

/******************************************************************************/

#ifdef TIMER0_USED_BY_RTI
#pragma vector = TIMER0_A0_VECTOR             //Interval timer vector location
__interrupt void rti_isr_timerA0(void)
{
    recorrerCallbacks();
}
#endif

/******************************************************************************/

#ifdef TIMER1_USED_BY_RTI
#pragma vector = TIMER1_A0_VECTOR             //Interval timer vector location
__interrupt void rti_isr_timerA1(void)
{
    recorrerCallbacks();
}
#endif
