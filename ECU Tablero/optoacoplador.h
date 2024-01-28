/***************************************************************************//**
  @file     optoacoplador.h
  @brief    optocoupler services.
  @authors  Capparelli, Mujica Buj, Olivera, Torres, Zannier Diaz
 ******************************************************************************/

#ifndef OPTOACOPLADOR_H_INCLUDED
#define OPTOACOPLADOR_H_INCLUDED

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include "common.h"
#include "rti.h"

/*******************************************************************************
 * ENUMERATIONS, STRUCTURES AND TYPEDEFS
 ******************************************************************************/
enum opto_out_pin{P1_1_OP=1, P1_2_OP, P1_5_OP, P1_6_OP, P2_6_OP,\
                   P2_0_OP, P2_1_OP, P2_2_OP, P2_3_OP, P2_4_OP, P2_5_OP};

/*******************************************************************************
 * PUBLIC FUNCTION PROTOTYPES
 ******************************************************************************/

 //Inicializar timer, pwm y motor.
// Se elije trabajar con un periodo de 20ms. El duty cycle permitido va de 0.5% a 99.5%.
// timer: Timer_A0-->0, Timer_A1-->1
// velOpto_x10: en rpm, de 0 a DC_VEL_MAX*10 (en realidad del 0.5% al 99.5% de DC_VEL_MAX*10).
//                 La velocidad real será 1/10 de velocidad_x_10; se agrega ese 'x_10' para tener
//                 precision hasta un decimal de rpm.
// pinOptoPWM: usar sólo los del enum: P1.1, P1.2, P1.5, P1.6, P2.6 para Timer0_A
//                              P2.0, P2.1, P2.2, P2.3, P2.4, P2.5 para Timer1_A
// recomendados: P1.2, P1.6, P2.6, P2.1, P2.2
void optoInit(uint8_t pin, uint8_t timer_num, void (*callback_capture)(void), void (*rtiTimer)(rti_callback_t, unsigned int));

//Modifica el registro interno de la velocidad medida, segun la medicion por Input Capture
void optoUpdate(void);

//Devuelve el registro interno de la velocidad medida
uint16_t readOptoVal(void);

//Sobreescribo el registro interno de la velocidad medida
void writeOptoVal(uint16_t newVelOpto);

//Modifica el registro interno de la velocidad medida y lo devuelve
uint16_t readOpto(void);

//Este driver soporta un solo optoacoplador.
/******************************************************************************/

#endif // OPTOACOPLADOR_H_INCLUDED
