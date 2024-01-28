/***************************************************************************//**
  @file     common.h
  @brief    Common types, definitions and macros.
  @author   Nicolás Magliola
 ******************************************************************************/

#ifndef _COMMON_H_
#define _COMMON_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include <stdint.h> //tipos de datos como uint8_t, etc y sus valores maximos
#include <stdbool.h> //TRUE, FALSE y no mucho mas (aunque casi ni lo uso, uso mis macros TRUE, FALSE
#include <stdio.h> //printf y sus variantes, y varias cosas mas
#include <msp430.h> //registros y defines para configurarlos

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/
//Se debe elegir qué módulo se queda con el Timer0_A y el Timer1_A
#define TIMER0_USED_BY_RTI //usar este para tablero
//#define TIMER0_USED_BY_HWT //usar este para motor
//#define TIMER1_USED_BY_RTI
#define TIMER1_USED_BY_HWT

//*** common constants **********************
#define MICROSECONDS_IN_MILISECOND  1000UL
#define MILISECONDS_IN_SECOND       1000UL
#define SECONDS_IN_MINUTE           60UL
#define MINUTES_IN_HOUR             60UL

#define BITS_IN_BYTE    8

#define FALSE           0
#define TRUE            1

#ifndef NULL
#define NULL            ((void*)0)
#endif // NULL


//*** useful macros **********************
#ifndef BITSET
#define BITSET(d,b) ((d) |= 1U << (b))
#define BITCLR(d,b) ((d) &= ~(1U << (b)))
#define BITTGL(d,b) ((d) ^= 1U << (b))
#define BITCHK(d,b) (0 != ((d) & (1U << (b))))
#define BOOLEAN(v)  (0 != (v))
#endif // BITSET

#define LOBYTE(w)       (0x00FFU&(w))
#define HIBYTE(w)       (((uint16_t)(w))>>BITS_IN_BYTE)

#define INCTRUNC(d, m)  do { if ((d)<(m)) ++(d); } while (0)
#define DECTRUNC(d, m)  do { if ((d)>(m)) --(d); } while (0)
#define INCMOD(d, m)    do { d = ((d) < (m))? ((d)+1) : 0; } while (0)
#define DECMOD(d, m)    do { d = ((d) > 0)? ((d)-1) : ((m)-1); } while (0)

#define NUMEL(v)        (sizeof(v)/sizeof(*(v)))

//*** delays **********************
#define F_CPU       1000000              //For a frequency of 1 MHz

#define DELAY_1s    F_CPU                // For __delay_cycles() ...
#define DELAY_500ms (F_CPU / 2)          // ...
#define DELAY_100ms (F_CPU / 10)         // ...
#define DELAY_10ms  (F_CPU / 100)        // ...
#define DELAY_1ms   (F_CPU / 1000)       // ...
#define DELAY_100us (F_CPU / 10000)      // ...
#define DELAY_10us  (F_CPU / 100000)     // ...
#define DELAY_1us   (F_CPU / 1000000)    // .-

/******************************************************************************/

#endif // _COMMON_H_
