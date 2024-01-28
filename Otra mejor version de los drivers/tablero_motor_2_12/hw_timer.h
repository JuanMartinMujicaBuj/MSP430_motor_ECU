/***************************************************************************//**
  @file     hw_timer.h
  @brief    Hardware timer services.
  @authors  Jacoby, Mujica Buj, Olivera, Torres, Zannier Diaz
 ******************************************************************************/

#ifndef HW_TIMER_H_
#define HW_TIMER_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include "common.h"

/*******************************************************************************
 * ENUMERATIONS, STRUCTURES AND TYPEDEFS
 ******************************************************************************/
enum hw_timer_mode{INPUT_CAPTURE_HWT, OUTPUT_COMPARE_HWT};
enum hw_timer_count_mode{UP_MODE_HWT, CONT_MODE_HWT, UP_DOWN_MODE_HWT};
enum hw_timer_output_mode{OUTPUT_MODE_HWT, SET_MODE_HWT, TOGGLE_RESET_MODE_HWT,\
                          SET_RESET_MODE_HWT, TOGGLE_MODE_HWT, RESET_MODE_HWT,\
                          TOGGLE_SET_MODE_HWT, RESET_SET_MODE_HWT};
enum hw_timer_out_pin{P1_1_HWT=1, P1_2_HWT, P1_5_HWT, P1_6_HWT, P2_6_HWT,\
                      P2_0_HWT, P2_1_HWT, P2_2_HWT, P2_3_HWT, P2_4_HWT, P2_5_HWT};
enum hw_timer_capture_mode{NO_CAPTURE, CAPTURE_ON_RISING_EDGE,CAPTURE_ON_FALLING_EDGE,\
                           CAPTURE_ON_BOTH_EDGES};
/*******************************************************************************
 * PUBLIC FUNCTION PROTOTYPES
 ******************************************************************************/
//Inicializar Timer_A: elegir clocks.
// timer_num: qué timer quiero usar. Timer0_A-->0, Timer1_A--> 1
void hwTimerInit(uint8_t timer_num);


//Iniciar timer:
//timer_num: qué timer quiero usar. Timer0_A-->0, Timer1_A--> 1

//timer_mode: input capture (para usar un optoacoplador / medir tiempos)
//            output compare (para hacer pwm)

//count_mode: up (sube hasta ccr0 y repite),
//            continuous (sube hasta 0xffff ticks y repite),
//            up-down (sube hasta ccr0, baja hasta 0 y repite).

//capture_mode (para input capture):
//            no capture (no captura nada, seria un "modo stand by")
//            rising edge (captura cuando el pin seleccionado tiene un rising edge)
//            falling edge (captura cuando el pin seleccionado tiene un falling edge)
//            both edges (captura en rising edge y tambien en falling edge)

//out_mode (para output compare):
//            output (no hace nada, o pone OUTx en el pin de salida, not sure),
//            set (setea el pin elegido al llegar a ccr1),
//            toggle-reset (togglea al llegar a ccr1, resetea al llegar a ccr0),
//            set-reset (setea en ccr1, resetea en ccr0),
//            toggle (togglea en ccr1),
//            reset (resetea en ccr1),
//            toggle-set (togglea en ccr1, setea en ccr0),
//            reset-set (resetea en ccr1, setea en ccr0).

//pin: para Timer0_A: P1.1, P1.2, P1.5, P1.6, P2.6 (entrar a la funcion con los valores del enum). pin=0 si no se quiere ninguno.
//     para Timer1_A: P2.0, P2.1, P2.2, P2.3, P2.4, P2.5 (entrar a la funcion con los valores del enum). pin=0 si no se quiere ninguno.
// Para output compare:
// P1.1, P1.5, P2.0, P2.3 son Out0 (no usar con set-reset, reset-set, toggle-set, toggle-reset)
// P1.2, P1.6, P2.1, P2.2, P2.6 son Out1 (recomendados)
// P2.4, P2.5 son Out2 (no programados, creo)
// Para input capture:
// P1.1, P2.0 son CCI0A (se pueden usar con CCR0, no se puede si ya se usa CCR0 para otra cosa como count up mode)
// P2.3 es CCI0B (idem CCI0A)
// P1.2, P2.1 son CCI1A (recomendados)
// P2.2 es CCI1B (tb recomendado)
// P2.4 es CCI2A (tb recomendado)
// P2.5 es CCI2B (tb recomendado)

//ccr0: valor en ticks (us) del TA0CCR0 deseado. De 0 a 65535.
//ccr1: valor en ticks (us) del TA0CCR1 deseado. De 0 a 65535. Usado solo en output compare mode.
//callback_ccr0: funcion de callback a la que llamar en la interrupcion del ccr0. NULL si no se quiere ninguna.
//callback_ccr1: funcion de callback a la que llamar en la interrupcion del ccr1. NULL si no se quiere ninguna.
void hwSetTimer(uint8_t timer_num, uint8_t timer_mode, uint8_t count_mode, uint8_t capture_mode, uint8_t out_mode, uint8_t pin, uint16_t ccr0, uint16_t ccr1, void (*callback_ccr0)(void), void (*callback_ccr1)(void));

//Devuelve ticks del free running counter del timer elegido (aka TA0R o TA1R), para lo que sea que se quiera.
uint16_t hwTimerGetTicks(uint8_t timer_num);

//incrementar ccr0 en la cantidad deseada. Util para armar una posible funcion de callback_ccr0:
//void callback_1(void){ hwTimerIncCCR0(1500); }, por ejemplo, con un 'num' hardcodeado.
void hwTimerIncCCR0(uint16_t num, uint8_t timer_num);

//incrementar ccr1 en la cantidad deseada. Util para armar una posible funcion de callback_ccr1.
void hwTimerIncCCR1(uint16_t num, uint8_t timer_num);

//Darle a ccr0 el valor deseado. Util para armar una posible funcion de callback_ccr0.
void hwTimerSetCCR0(uint16_t num, uint8_t timer_num);

//Darle a ccr1 el valor deseado. Util para armar una posible funcion de callback_ccr1.
void hwTimerSetCCR1(uint16_t num, uint8_t timer_num);

//Leer valor del CCR0 (util para input capture)
uint16_t hwTimerGetCCR0(uint8_t timer_num);

//Leer valor del CCR1 (util para input capture)
uint16_t hwTimerGetCCR1(uint8_t timer_num);

//Leer valor del CCR2 (util para input capture)
uint16_t hwTimerGetCCR2(uint8_t timer_num);


//Frenar el timer. Count mode: stop mode
void hwStopTimer(uint8_t timer_num);

//Este driver sirve para setear Timer0_A y/o Timer1_A.
//Usar en modo Output Compare, para pwm o interrupts periodicas,
//o Input Capture, para captar eventos de duracion de microsegs.

//Muchísima atención a los pines que se usan!

//Al no setear nunca ccr2, creo que me pierdo de poder hacer outmod con ccr2.
//Es una posible ampliacion. Podría hacer que sea igual que ccr1 pero no
//habilitarle interrupts, asi puedo hacer outmod a todos los pines Out2 pero
//solo ejecuto el callback que le cargo a ccr1. El problema es que ademas de ccr1,
//tendria que actualizar siempre ccr2.

//Para input capture, para aprovechar la separacion entre ccr0 callback y ccr1 callback
//que tiene este driver, una opcion es usar count up mode, poner logica de overflow en
//el ccr0 callback, y logica de input capture en el ccr1. Esto conviene porque el
//interrupt de overflow esta en el mismo vector de interrupcion que el del ccr1 y ccr2.
//Si no, dentro del callback de ccr1 tendria que andar mirando si la interrupcion la
//activo el overflow o el ccr1. La otra opcion es usar ccr0 callback para la logica
//de input capture y ccr1 callback para la logica de overflow.

//Si en input capture se usa continuous mode, los valores de ccr0, ccr1 que se ingresan
//no se usan para nada. Si se usa up mode (o up-down mode, que no tiene mucho sentido),
//se toma el valor de ccr0, y se descarta el de ccr1.

//Para input capture, sí o sí necesito funciones de callback de ccr0 y ccr1, que se
//hagan cargo de la logica de tiempo y overflow (o de contar pulsos, etc).

//Como ampliacion, podrian usarse distintos CCRx para distintas acciones.
/******************************************************************************/

#endif /* HW_TIMER_H_ */
