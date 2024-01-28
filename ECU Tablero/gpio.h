/***************************************************************************//**
  @file     gpio.h
  @brief    Simple GPIO Pin services, similar to Arduino.
  @author   Nicolás Magliola
 ******************************************************************************/

#ifndef _GPIO_H_
#define _GPIO_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include "common.h"

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/
#define LOW             0
#define HIGH            1

#ifndef INPUT
#define INPUT           0
#define OUTPUT          1
#define INPUT_PULLUP    2
#define INPUT_PULLDOWN  3
#endif // INPUT

// Convert port and numbber into pin ID
// Ex: P1.4 -> PORTNUM2PIN(1, 4) --> 0x14
//     P2.6 -> PORTNUM2PIN(2, 6) --> 0x26
#define PORTNUM2PIN(po, n)  ((po)<<4 | (n))
#define PIN2PORT(pi)        (((pi)>>4) & 0x03)
#define PIN2NUM(pi)         ((pi) & 0x07)

/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/
typedef uint8_t gpio_t;

/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/
/* @brief Configures the specified pin to behave as a GPIO
 * @param pin The pin id whose mode you wish to set (according PORTNUM2PIN)
 * @param mode INPUT, OUTPUT, INPUT_PULLUP or INPUT_PULLDOWN.
 */
void gpioMode (gpio_t pin, uint8_t mode);

/* @brief Writes a HIGH or a LOW value to a digital pin
 * @param pin The pin id (according PORTNUM2PIN)
 * @param value Desired value (HIGH or LOW)
 */
void gpioWrite (gpio_t pin, uint8_t value);

/* @brief Toggles the value of a digital pin (HIGH<->LOW)
 * @param pin The pin id (according PORTNUM2PIN)
 */
void gpioToggle (gpio_t pin);

/* @brief Reads the value from a specified digital pin, either HIGH or LOW.
 * @param pin The pin id (according PORTNUM2PIN)
 * @return HIGH or LOW
 */
uint8_t gpioRead (gpio_t pin);

/* @brief Writes an 8-bit number to a digital port.
 * @param port The port id
 * @param value Desired value (i.e. 0b11001100)
 */
void gpioWritePort (uint8_t port, uint8_t value);

/* @brief Writes a 7-bit number to pins 0-6 of a digital port.
 * @param port The port id
 * @param value 7-bit desired value (i.e. 0b11001100)
 */
void gpioWrite7bit(uint8_t port, uint8_t value_7bit);
/******************************************************************************/

#endif // _GPIO_H_
