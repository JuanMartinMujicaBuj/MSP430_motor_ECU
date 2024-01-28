/***************************************************************************//**
  @file     gpio.c
  @brief    Simple GPIO Pin services, similar to Arduino.
  @author   Nicolás Magliola
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include "gpio.h"

/*******************************************************************************
* GLOBAL FUNCTION DEFINITIONS
*******************************************************************************/
void gpioMode (gpio_t pin, uint8_t mode)
{
    uint8_t port, num;

    port = PIN2PORT(pin);
    num = PIN2NUM(pin);

    switch (port)
    {
    case 1:
        BITCLR(P1SEL , num);
        BITCLR(P1SEL2, num);
        if (mode == OUTPUT)
        {
            BITSET(P1DIR, num); // OUTPUT
        }
        else
        {
            BITCLR(P1DIR, num); // INPUT
            if (mode == INPUT)
            {
                BITCLR(P1REN, num); // pull disable
            }
            else
            {
                BITSET(P1REN, num); // pull enable
                if (mode == INPUT_PULLDOWN)
                {
                    BITCLR(P1OUT, num); // pull-down
                }
                else
                {
                    BITSET(P1OUT, num); // pull-up
                }
            }
        }
        break;

    case 2:
        BITCLR(P2SEL , num);
        BITCLR(P2SEL2, num);
        if (mode == OUTPUT)
        {
            BITSET(P2DIR, num); // OUTPUT
        }
        else
        {
            BITCLR(P2DIR, num); // INPUT
            if (mode == INPUT)
            {
                BITCLR(P2REN, num); // pull disable
            }
            else
            {
                BITSET(P2REN, num); // pull enable
                if (mode == INPUT_PULLDOWN)
                {
                    BITCLR(P2OUT, num); // pull-down
                }
                else
                {
                    BITSET(P2OUT, num); // pull-up
                }
            }
        }
        break;
    }
}

/******************************************************************************/

void gpioWrite (gpio_t pin, uint8_t value)
{
    uint8_t port, num;

    port = PIN2PORT(pin);
    num = PIN2NUM(pin);

    switch (port) // set or clear bit from proper PxOUT
    {
    case 1:
        if (value)
            BITSET(P1OUT, num);
        else
            BITCLR(P1OUT, num);
        break;

    case 2:
        if (value)
            BITSET(P2OUT, num);
        else
            BITCLR(P2OUT, num);
        break;
    }
}

/******************************************************************************/

void gpioWritePort (uint8_t port, uint8_t value)
{
    switch (port)
    {
    case 1:
        P1OUT = value;
        break;

    case 2:
        P2OUT = value;
        break;
    }
}

/******************************************************************************/

void gpioWrite7bit(uint8_t port, uint8_t value_7bit)
{ //Writes bits 0-6 of the selected port (useful for 7seg displays)
    switch (port)
    {
    case 1:
        P1OUT &= BIT7; //clear bits 0-6
        P1OUT |= value_7bit; //set bits 0-6
        break;

    case 2:
        P2OUT &= BIT7; //clear bits 0-6
        P2OUT |= value_7bit; //set bits 0-6
        break;
    }
}

/******************************************************************************/

void gpioToggle (gpio_t pin)
{
    uint8_t port, num;

    port = PIN2PORT(pin);
    num = PIN2NUM(pin);

    switch (port) // toggle bit from proper PxOUT
    {
    case 1:
        BITTGL(P1OUT, num);
        break;

    case 2:
        BITTGL(P2OUT, num);
        break;
    }
}

/******************************************************************************/

uint8_t gpioRead (gpio_t pin)
{
    uint8_t port, num, value;

    port = PIN2PORT(pin);
    num = PIN2NUM(pin);

    switch (port) // check bit from proper PxIN
    {
    case 1:
        value = BITCHK(P1IN, num);
        break;

    case 2:
        value = BITCHK(P2IN, num);
        break;

    default:
        value = LOW;
        break;
    }

    return value;
}
