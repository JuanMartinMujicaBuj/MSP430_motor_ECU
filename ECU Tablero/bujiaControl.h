/***************************************************************************//**
  @file     bujia.h
  @brief    Spark plug (bujía) services. Switch on or off according to motor speed
            and start and stop angles.
  @authors  Mujica Buj
 ******************************************************************************/

#ifndef BUJIACONTROL_H_
#define BUJIACONTROL_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include "bujia.h"

/*******************************************************************************
 * PUBLIC FUNCTION PROTOTYPES
 ******************************************************************************/
// Inicializar driver de bujía. Guardar velocidad de giro y angulo de encendido
// y apagado iniciales. Suscribirse a interrupts periodicas, en las que actualizar
// el angulo y el estado (on-off) de la salida (la bujia).
// pin: usar PORTNUM2PIN() del common. Por ejemplo, para P1.2-->PORTNUM2PIN(1,2)
// rpm: velocidad de giro del motor en rpm
// phi1: ángulo (en grados) de encendido de la bujía
// phi2: ángulo (en grados) de apagado de la bujía
void bujiaControlInit(uint8_t pin, uint8_t rpm, uint16_t phi1, uint16_t phi2);

// Modificar los rpm, phi1 y/o phi2
void bujiaControlWrite(uint8_t rpm, uint16_t phi1, uint16_t phi2);

// Cerar (poner en 0) el ángulo de giro (el que se actualiza en las interrupts
// periodicas y se usa para decidir si corresponde prender o apagar la bujia)
void bujiaZero(void);

// Poner en cierto valor el ángulo de giro (el que se actualiza en las interrupts
// periodicas y se usa para decidir si corresponde prender o apagar la bujia).
// Util para ajustar cada cierto tiempo la posicion del angulo, que puede ir desviandose.
void bujiaSetAngle(uint16_t phi);

//Desactivar la bujia
void bujiaStop(void);

//Este driver soporta una sola bujía. Puede ampliarse a más.
//Para 250 rpm, cada grado tarda 0.66ms-->sería apropiado trabajar con interrupts
//periodicas cada 0.5ms.
/******************************************************************************/
#endif /* BUJIACONTROL_H_ */
