/***************************************************************************//**
  @file     adc_2.h
  @brief    ADC services. Simpler than adc.h, works without rti's.
  @authors  Mujica Buj
 ******************************************************************************/

#ifndef ADC_2_H_
#define ADC_2_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include "common.h"

/*******************************************************************************
 * PUBLIC FUNCTION PROTOTYPES
 ******************************************************************************/
//Inicializa un pin para ADC (entrada analogica).
// pin: para analog, se usa siempre el puerto 1 (P1.x).
void adcInit(uint8_t pin);

//Realizar una lectura (sample&hold del adc)
uint16_t adcRead(void);

//Funciona para una sola entrada analógica. Se puede ampliar a más (implica
//cambiar el modo del adc, ver datasheet).
/******************************************************************************/
#endif /* ADC_2_H_ */
