/***************************************************************************//**
  @file     bujia.h
  @brief    Spark plug (bujía) services.
  @authors  Capparelli, Mujica Buj, Olivera, Torres, Zannier
 ******************************************************************************/

#ifndef BUJIA_H_INCLUDED
#define BUJIA_H_INCLUDED

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include "common.h"
#include "rti.h"

/*******************************************************************************
 * PUBLIC FUNCTION PROTOTYPES
 ******************************************************************************/
// Inicializar driver de bujía. Guardar velocidad de giro y angulo de encendido
// y apagado iniciales.
// pinBujia: usar PORTNUM2PIN() del common. Por ejemplo, para P1.2-->PORTNUM2PIN(1,2)

void bujiaInit(uint8_t pinBujia, void (*rtiTimer)(rti_callback_t, unsigned int));

//Modifica estado de la bujia (on/off)
void bujiaUpdate(void);

//Devuelve el registro con el estado de la bujia
uint8_t readBujiaVal(void);

//Sobreescribe el registro con el estado de la bujia
void writeBujiaVal(uint8_t bujiaStatus);

//Sobreescribe el registro con el estado de la bujia y setea la bujia a ese estado
void writeBujia(uint8_t bujiaStatus);

//Este driver soporta una sola bujía.
/******************************************************************************/


#endif // BUJIA_H_INCLUDED
