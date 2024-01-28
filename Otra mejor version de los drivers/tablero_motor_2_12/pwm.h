/***************************************************************************//**
  @file     pwm.h
  @brief    PWM services.
  @authors  Capparelli & Mujica Buj
 ******************************************************************************/

#ifndef PWM_H_
#define PWM_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include "common.h"

/*******************************************************************************
 * ENUMERATIONS, STRUCTURES AND TYPEDEFS
 ******************************************************************************/
enum pwm_mode{HIGH_LOW_PWM, LOW_HIGH_PWM, CENTERED_PWM};
enum pwm_out_pin{P1_1_PWM=1, P1_2_PWM, P1_5_PWM, P1_6_PWM, P2_6_PWM,\
                 P2_0_PWM, P2_1_PWM, P2_2_PWM, P2_3_PWM, P2_4_PWM, P2_5_PWM};

/*******************************************************************************
 * PUBLIC FUNCTION PROTOTYPES
 ******************************************************************************/
//Inicializar timer. Inicializar pwm: elegir timer (A0 o A1), modo
//(HIGH_LOW, LOW_HIGH, CENTERED), tiempo total del periodo y tiempo encendido.
// Usa por defecto el Timer0_A.
// timer: Timer_A0-->0, Timer_A1-->1
// modo: HIGH_LOW_PWM, LOW_HIGH_PWM, CENTERED_PWM
// t_periodo: en microsegundos
// t_on: en microsegundos
// pin: usar sólo los del enum: P1.1, P1.2, P1.5, P1.6, P2.6 con Timer0_A
//                              P2.0, P2.1, P2.2, P2.3, P2.4, P2.5 con Timer1_A
// recomendados: P1.2, P1.6, P2.6, P2.1, P2.2 (los otros pueden no funcionar si entendí bien el manual)
void pwmInit(uint8_t timer_num, uint8_t modo, uint16_t t_periodo, uint16_t t_on, uint8_t pin);

//Cambiar tiempo total del periodo y tiempo encendido.
void pwmWrite(uint8_t timer_num, uint16_t t_periodo, uint16_t t_on);

//Frenar el pwm
void pwmStop(uint8_t timer_num);

//Este driver soporta hasta 2 pwm's: uno con el Timer0_A, otro con el Timer1_A.
//El periodo maximo es de 65ms. Puede fallar para duty cycles muy chicos o muy grandes
//(si entre ccr0 y ccr1 hay una diferencia de unos pocos microseg).
//Por ahora, sólo está programado el modo HIGH_LOW.
/******************************************************************************/
#endif /* PWM_H_ */
