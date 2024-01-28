/***************************************************************************//**
  @file     rti.c
  @brief    Periodic interrupts and callback management.
  @author   Mujica Buj
 ******************************************************************************/

#ifndef RTI_H_
#define RTI_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include "common.h"

/*******************************************************************************
 * ENUMERATIONS, STRUCTURES AND TYPEDEFS
 ******************************************************************************/
typedef void (*rti_callback_t)(void);

/*******************************************************************************
 * PUBLIC FUNCTION PROTOTYPES
 ******************************************************************************/
//Guarda función de callback de otro módulo en el vector de callbacks, junto con
//su frecuencia de llamado. IMPORTANTE: guardar primero los callbacks que es mas
//importante que se ejecuten en tiempo (ej: el de displays de array7seg). Devuelve
//un id de la funcion de callback, por si en el futuro se la quisiera borrar.
uint8_t rtiSubmitCallback (rti_callback_t fun, uint16_t fun_frequency);

//Borra la funcion de callback con el callback_id ingresado, de la lista de
//callbacks que tiene el rti.
void rtiClearCallback (uint8_t callback_id);

/******************************************************************************/

#endif /* RTI_H_ */
