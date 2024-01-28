/***************************************************************************//**
  @file     motor.h
  @brief    motor services: pwm.
  @authors  Capparelli, Mujica Buj, Olivera, Torres, Zannier
 ******************************************************************************/

#ifndef MOTOR_H_INCLUDED
#define MOTOR_H_INCLUDED

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include "common.h"
#include "rti.h"

/*******************************************************************************
 * ENUMERATIONS, STRUCTURES AND TYPEDEFS
 ******************************************************************************/
enum motor_out_pin{P1_1_DC=1, P1_2_DC, P1_5_DC, P1_6_DC, P2_6_DC,\
                   P2_0_DC, P2_1_DC, P2_2_DC, P2_3_DC, P2_4_DC, P2_5_DC};

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/
#define DC_VEL_MAX 216 //en rpm. Ajustable segun el motor. El actual: 6V, ~216rpm.

/*******************************************************************************
 * PUBLIC FUNCTION PROTOTYPES
 ******************************************************************************/
//Inicializar timer, pwm y motor.
// Se elije trabajar con un periodo de 20ms. El duty cycle permitido va de 0.5% a 99.5%.
// timer: Timer_A0-->0, Timer_A1-->1
// velMotor_x10: en rpm, de 0 a DC_VEL_MAX*10 (en realidad del 0.5% al 99.5% de DC_VEL_MAX*10).
//                 La velocidad real será 1/10 de velocidad_x_10; se agrega ese 'x_10' para tener
//                 precision hasta un decimal de rpm.
// pin: usar sólo los del enum: P1.1, P1.2, P1.5, P1.6, P2.6 para Timer0_A
//                              P2.0, P2.1, P2.2, P2.3, P2.4, P2.5 para Timer1_A
// recomendados: P1.2, P1.6, P2.6, P2.1, P2.2

//Inicializacion del Driver
void motorInit(uint8_t pinMotorPWM, uint8_t timer_num, void (*rtiTimer)(rti_callback_t, unsigned int));

//Modifica la velocidad del motor (modificando la señal PWM de salida) segun el registro de velocidad guardado
void motorUpdate(void);

//Devuelve el registro de velocidad guardado
uint8_t readVelMotor(void);

//Sobreescribe el registro de velocidad guardado
void writeVelMotor(uint16_t newVelMotor);

//Modifica el regustro de velocidad guardado y la velocidad del motor
void writeMotor(uint16_t newVelMotor);

//Frenar/apagar el motor.
void motorStop(void);

//Este driver soporta 1 motor dc.
/******************************************************************************/

#endif // MOTOR_H_INCLUDED

