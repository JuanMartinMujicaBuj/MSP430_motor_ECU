/***************************************************************************//**
  @file     inputCapture.h
  @brief    input capture services using a timer.
  @authors  Mujica Buj, Olivera
 ******************************************************************************/

#ifndef INPUTCAPTURE_H_
#define INPUTCAPTURE_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include "common.h"

/*******************************************************************************
 * ENUMERATIONS, STRUCTURES AND TYPEDEFS
 ******************************************************************************/
enum ic_timer_out_pin{P1_1_IC=1, P1_2_IC, P1_5_IC, P1_6_IC, P2_6_IC,\
                      P2_0_IC, P2_1_IC, P2_2_IC, P2_3_IC, P2_4_IC, P2_5_IC};
enum ic_timer_capture_mode{NO_CAPTURE_IC, CAPTURE_ON_RISING_EDGE_IC,CAPTURE_ON_FALLING_EDGE_IC,\
                           CAPTURE_ON_BOTH_EDGES_IC};

/*******************************************************************************
 * PUBLIC FUNCTION PROTOTYPES
 ******************************************************************************/
//Inicializar un timer en modo input capture. Para este driver, usa count up mode
// y pines: los que trabajan con CCR1 o CCR2.
//Puede detectar flancos ascendentes, descendentes o ambos.
//En las interrupciones de capture y overflow, en las que ya ejecuta una logica de capture y
//de overflow respectivamente, permite agregar callbacks extra.
//pin: para Timer0_A: P1.1, P1.2, P1.5, P1.6, P2.6 (entrar a la funcion con los valores del enum).
//     para Timer1_A: P2.0, P2.1, P2.2, P2.3, P2.4, P2.5 (entrar a la funcion con los valores del enum).
// P1.1, P2.0 son CCI0A (se pueden usar con CCR0, no se puede si ya se usa CCR0 para otra cosa como count up mode)
// P2.3 es CCI0B (idem CCI0A)
// P1.2, P2.1 son CCI1A (recomendados)
// P2.2 es CCI1B (tb recomendado)
// P2.4 es CCI2A (no codeado)
// P2.5 es CCI2B (no codeado)
void inputCaptureInit(uint8_t pin, uint8_t timer_num_in, uint8_t capture_mode, void (*callback_overflow)(void), void (*callback_capture)(void));

//Leer tiempo entre dos captures, en milisegundos
uint16_t inputCaptureRead(void);

//Este driver soporta un solo input capture y tiempos de hasta 10 segundos entre captures (y un poco mas tambien).
/******************************************************************************/

#endif /* INPUTCAPTURE_H_ */
