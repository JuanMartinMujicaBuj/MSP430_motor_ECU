/***************************************************************************//**
  @file     servo.h
  @brief    Servo services.
  @authors  Capparelli & Mujica Buj
 ******************************************************************************/

#ifndef SERVO_H_
#define SERVO_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include "common.h"

/*******************************************************************************
 * ENUMERATIONS, STRUCTURES AND TYPEDEFS
 ******************************************************************************/
enum servo_out_pin{P1_1_SERVO=1, P1_2_SERVO, P1_5_SERVO, P1_6_SERVO, P2_6_SERVO,\
                   P2_0_SERVO, P2_1_SERVO, P2_2_SERVO, P2_3_SERVO, P2_4_SERVO, P2_5_SERVO};

/*******************************************************************************
 * PUBLIC FUNCTION PROTOTYPES
 ******************************************************************************/
//Inicializar timer, pwm y servo.
// El servo trabaja con un periodo de 20ms, con un t_on de 0.5ms para -90° a
// 2.5ms para +90° (micro servo 9g)
// timer: Timer_A0-->0, Timer_A1-->1
// angulo: -90° a +90°
// pin: usar sólo los del enum: P1.1, P1.2, P1.5, P1.6, P2.6 para Timer0_A
//                              P2.0, P2.1, P2.2, P2.3, P2.4, P2.5 para Timer1_A
// recomendados: P1.2, P1.6, P2.6, P2.1, P2.2
void servoInit(uint8_t timer_num, int8_t angulo, uint8_t pin);

//Cambiar angulo.
void servoWrite(int8_t angulo);

//Frenar/apagar el servo.
void servoStop(void);

//Este driver soporta 1 servo.
/******************************************************************************/

#endif /* SERVO_H_ */
