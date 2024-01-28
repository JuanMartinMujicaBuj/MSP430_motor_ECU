/***************************************************************************//**
  @file     spi.c
  @brief    SPI serial communication services. Adapted from mcp2515 library
            by K. Evangelos.
  @authors  Mujica Buj
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include "spi.h"

/*******************************************************************************
* GLOBAL FUNCTION DEFINITIONS
*******************************************************************************/
// Initialize SPI-UCB0 for Texas Instruments MSP430G2553 microcontrollers
// (bear in mind that MSP430G2553 also has UCA0 for SPI).
void SPI_init(void)
{
  P1SEL |= BIT5 + BIT6 + BIT7;  //Uses UCB0 for SPI communication
  P1SEL2 |= BIT5 + BIT6 + BIT7; //(UCA0 is reserved for uart)

  P2DIR |= BIT5; //chip select pin                                       // !CS-Leitung (beachte auch Defines MCP2515_CS_LOW und MCP2515_CS_High)  ...
  P2OUT |= BIT5;                                                         // .-

  UCB0CTL1 |= UCSWRST;                                                   // Reset. Inactive state high, MSB first, master mode, 3-pin, synchronous mode,
  UCB0CTL0 |= UCCKPL + UCMSB + UCMST + UCMODE_0 + UCSYNC;                // 8-bit SPI, data captured on the first edge and changed on the following one
  UCB0CTL1 |= UCSSEL_2;                                                  // SMCLK
  UCB0BR0 |= 0x02; //Baud rate of 500.000 for BRCLK frequency of 1MHz    // /2 ...
  UCB0BR1 = 0;                                                           // .-
  //UCA0MCTL = 0;                                                        // No modulation
  UCB0CTL1 &= ~UCSWRST;                                                  // Initialize USCI state machine

  __delay_cycles(DELAY_100ms);                                           // Warte 100ms
  while (!(IFG2 & UCB0TXIFG));                                           // Warte bist übermittelt
  UCB0TXBUF = 0x00;                                                      // Dummy Senden
  __delay_cycles(DELAY_100ms);
}

/******************************************************************************/
// Send and receive via SPI-UCB0
// Variables
// @ daten    : Unsigned 8-Bit-Datensatz der über UCB0 via SPI gesendet werden soll
// @ rückgabe : Unsigned 8-Bit-Datensatz der über UCB0 via SPI empfangen wird
unsigned char SPI_transmit(unsigned char daten)
{
  UCB0TXBUF = daten;            // Sende Datensatz
  while(UCB0STAT & UCBUSY);     // Warte bist übermittelt
  return UCB0RXBUF;             // Gebe empfangenen Datensatz zurück
}

/******************************************************************************/
