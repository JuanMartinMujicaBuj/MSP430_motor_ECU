/***************************************************************************//**
  @file     servo.h
  @brief    Servo services.
  @authors  Capparelli, Mujica Buj, Olivera, Torres, Zannier
 ******************************************************************************/

#ifndef SERVO_H_INCLUDED
#define SERVO_H_INCLUDED

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include "common.h"
#include "rti.h"

/*******************************************************************************
 * ENUMERATIONS, STRUCTURES AND TYPEDEFS
 ******************************************************************************/
enum servo_out_pin{P1_1_SERVO=1, P1_2_SERVO, P1_5_SERVO, P1_6_SERVO, P2_6_SERVO,\
                   P2_0_SERVO, P2_1_SERVO, P2_2_SERVO, P2_3_SERVO, P2_4_SERVO, P2_5_SERVO};

/*******************************************************************************
 * PUBLIC FUNCTION PROTOTYPES
 ******************************************************************************/
//Inicializar timer, pwm y servo.
// El servo trabaja con un periodo de 20ms, con un t_on de 1ms para -90° a
// 2ms para +90° (micro servo 9g SG90)
// timer: Timer_A0-->0, Timer_A1-->1
// pos: -90° a +90°
// pin: usar sólo los del enum: P1.1, P1.2, P1.5, P1.6, P2.6 para Timer0_A
//                              P2.0, P2.1, P2.2, P2.3, P2.4, P2.5 para Timer1_A
// recomendados: P1.2, P1.6, P2.6, P2.1, P2.2

//Inicializacion del Driver
void servoInit(uint8_t pinServoPWM, uint8_t timer_num, void (*rtiTimer)(rti_callback_t, unsigned int));

//Cambia el angulo del servo (cambiando el PWM) segun el registro de posicion guardado
void servoUpdate(void);

//Devuelve el registro de posicion guardado
int8_t readServoPos(void);

//Sobreescribe el registro de posicion guardado
void writeServoPos(int8_t newPos);

//Sobreeescribe el registro de posicion guardado y cambia el angulo del servo.
void servoWrite(int8_t angulo);

//Apaga servo. Vuelve a la posicion central.
void servoStop(void);

//Este driver soporta 1 servo.
/******************************************************************************/

#endif // SERVO_H_INCLUDED
