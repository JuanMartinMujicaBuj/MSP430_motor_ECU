/***************************************************************************//**
  @file     adc_2.c
  @brief    ADC services. Simpler than adc.c, works without rti's.
  @authors  Mujica Buj
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include "adc_2.h"
#include "gpio.h"

/*******************************************************************************
* GLOBAL FUNCTION DEFINITIONS
*******************************************************************************/
void adcInit(uint8_t pin)
{
    uint8_t num = PIN2NUM(pin); //Siempre puerto 1 para ADC. Se guarda el nro de pin.

    //Chequeo de que no se haya inicializado antes
    static uint8_t yaInit = 0;
    if (yaInit)
        return;
    yaInit = 1;

    ADC10CTL0 = SREF_0 + ADC10SHT_2 + ADC10ON;
    //SREF_0: reference: V+=Vcc, V-=Vss
    //ADC10SHT_2: sample and hold time: 16 x ADC10CLK's
    //sampling rate: reference buffer supports up to ~200ksps
    //reference output off
    //reference burst: reference buffer on continuously
    //Multiple sample and conversion (valid only for sequence or repeated modes, which
    //      are not being used): the sampling requires a rising edge of the SHI signal
    //      to trigger each sample-and-conversion.
    //reference generator voltage (if REFON is set, but it is not): 1.5V
    //reference generator: off
    //ADC10ON: ADC10 on
    //ADC10 interrupt disabled
    //no interrupt pending (ADC10IFG set when ADC10MEM is loaded with a conversion result)
    //ADC10 disabled (ENC: enable conversion)
    //no sample and conversion start (ADC10SC: start conversion)


    ADC10CTL1 = (num*0x1000u);
    // input Ax (with x=num), similar to INCH_x

    //sample and hold source select: ADC10SC bit
    //data format: straight binary
    //the sample-input signal is not inverted
    //ADC10 clock divider: /1
    //ADC10 clock source select: ADC10OSC (internal oscilator in the 5MHz range)
    //conversion sequence mode select: single-channel-single-conversion.
    //      este (CONSEQx) habria que tocar para leer mas de un analog input.
    //ADC10 busy: this bit indicates an active sample or conversion operation


    ADC10AE0 |= (1 << num);
    //Enable  P1.x pin as Analog input (Ax) (with x=num)
}

/******************************************************************************/

uint16_t adcRead(void)
{
    ADC10CTL0 |= ENC + ADC10SC; // Enable converter & Start of new conversion
    while(ADC10CTL1 & ADC10BUSY){
        //wait
    }
    return ADC10MEM;
}

/******************************************************************************/
