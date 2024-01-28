/***************************************************************************//**
  @file     dc_motor.h
  @brief    dc motor services: pwm.
  @authors  Mujica Buj
 ******************************************************************************/

#ifndef DC_MOTOR_H_
#define DC_MOTOR_H_


/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include "common.h"

/*******************************************************************************
 * ENUMERATIONS, STRUCTURES AND TYPEDEFS
 ******************************************************************************/
enum dc_motor_out_pin{P1_1_DC=1, P1_2_DC, P1_5_DC, P1_6_DC, P2_6_DC,\
                   P2_0_DC, P2_1_DC, P2_2_DC, P2_3_DC, P2_4_DC, P2_5_DC};

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/
#define DC_VEL_MAX 216 //en rpm. Ajustable segun el motor. El actual: 6V, ~216rpm.

/*******************************************************************************
 * PUBLIC FUNCTION PROTOTYPES
 ******************************************************************************/
//Inicializar timer, pwm y dc_motor.
// Se elije trabajar con un periodo de 20ms. El duty cycle permitido va de 0.5% a 99.5%.
// (se podria probar con mas...para otro dia).
// timer: Timer_A0-->0, Timer_A1-->1
// velocidad_x_10: en rpm, de 0 a DC_VEL_MAX*10 (en realidad del 0.5% al 99.5% de DC_VEL_MAX*10).
//                 La velocidad real será 1/10 de velocidad_x_10; se agrega ese 'x_10' para tener
//                 precision hasta un decimal de rpm.
// pin: usar sólo los del enum: P1.1, P1.2, P1.5, P1.6, P2.6 para Timer0_A
//                              P2.0, P2.1, P2.2, P2.3, P2.4, P2.5 para Timer1_A
// recomendados: P1.2, P1.6, P2.6, P2.1, P2.2
void dcMotorInit(uint8_t timer_num, uint16_t velocidad_x10, uint8_t pin);

//Cambiar velocidad.
void dcMotorWrite(uint16_t velocidad_x10);

//Frenar/apagar el motor.
void dcMotorStop(void);

//Este driver soporta 1 motor dc.
/******************************************************************************/

#endif /* DC_MOTOR_H_ */
