/***************************************************************************//**
  @file     system.h
  @brief    System initialization.
  @author   Magliola, Mujica Buj
 ******************************************************************************/

#ifndef SYSTEM_H_
#define SYSTEM_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include "common.h"

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/
#define enable_interrupts()     _BIS_SR(GIE)
#define disable_interrupts()    _BIC_SR(GIE)

/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/
enum clock_frec{MHZ_1, MHZ_8, MHZ_12, MHZ_16};
enum rti_period{MS_0064, MS_05, MS_8, MS_32, MS_1, MS_2, MS_5, MS_10, MS_50};
enum rti_timer{RTI_TIMER0, RTI_TIMER1, RTI_WDT, NO_RTI};
//Los valores de rti_period valen para SMCLK=1MHz. Para los primeros 4, se usa el WDT.
//Para los otros 5, se puede elegir entre el Timer0_A y el Timer1_A.
//Si no se quieren rti's, usar NO_RTI (y cualquier rti_period).

/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/
//Inicializa el sistema: frena el watchdog, elige y setea el clock a cierta
//frecuencia, setea interrupciones periodicas cada cierto periodo (para
//desactivar interrupts periodicos, rti_period_in_us=0).
void systemInit(uint8_t clock_frec_in_MHz, uint8_t rti_period_in_us, uint8_t rti_timer);

/******************************************************************************/

#endif /* SYSTEM_H_ */
