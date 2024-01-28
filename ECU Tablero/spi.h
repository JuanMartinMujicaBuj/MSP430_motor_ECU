/***************************************************************************//**
  @file     spi.h
  @brief    SPI serial communication services. Adapted from mcp2515 library
            by K. Evangelos.
  @authors  Mujica Buj
 ******************************************************************************/

#ifndef SPI_H_
#define SPI_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include "common.h"

/*******************************************************************************
 * PUBLIC FUNCTION PROTOTYPES
 ******************************************************************************/
void SPI_init(void);                             // Initialize SPI-UCB0
unsigned char SPI_transmit(unsigned char daten); // Send and receive via SPI-UCB0

/******************************************************************************/
#endif /* SPI_H_ */
