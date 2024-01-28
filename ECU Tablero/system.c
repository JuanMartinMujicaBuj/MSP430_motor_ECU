/***************************************************************************//**
  @file     system.c
  @brief    System initialization.
  @author   Magliola, Mujica Buj
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include "system.h"

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/
#define CCR0_1MS  (1000-1);  //ticks hasta los que debe contar el timer para
#define CCR0_2MS  (2000-1);  //interrumpir cada 1, 2, 5 o 10ms. Se resta 1
#define CCR0_5MS  (5000-1);  //porque el free running counter va por ej de
#define CCR0_10MS (10000-1); //0 a 999 para hacer 1000 ticks.
#define CCR0_50MS (50000-1);

/*******************************************************************************
 * PRIVATE FUNCTION PROTOTYPES
 ******************************************************************************/
void stopWDT(void);
void setCLK(uint8_t clock_frec_in_MHz);
void setRTI(uint8_t rti_period_in_us, uint8_t rti_timer);

/*******************************************************************************
* GLOBAL FUNCTION DEFINITIONS
*******************************************************************************/
void systemInit(uint8_t clock_frec_in_MHz, uint8_t rti_period_in_us, uint8_t rti_timer)
{
    //Chequeo de que no se haya inicializado antes
    static uint8_t yaInit = 0;
    if (yaInit)
        return;
    yaInit = 1;

    //Watchdog timer
    stopWDT();

    //Frecuencia y seleccion del clock (por defecto, MCLK=SMCLK=DCO)
    setCLK(clock_frec_in_MHz);

    //Interrupciones por Watchdog, Timer1_A o Timer0_A.
    if(rti_timer !=NO_RTI)
    {
        setRTI(rti_period_in_us, rti_timer);
    }
}

/*******************************************************************************
 * PRIVATE FUNCTION DEFINITIONS
 ******************************************************************************/
void stopWDT(void)
{
    WDTCTL = WDTPW | WDTHOLD; // Frena el watchdog timer
}

/******************************************************************************/

void setCLK(uint8_t clock_frec_in_MHz)
{
    switch(clock_frec_in_MHz)
        {
        case MHZ_1:
            BCSCTL1 = CALBC1_1MHZ; // Set range
            DCOCTL  = CALDCO_1MHZ; // Set DCO step + modulation
            break;
        case MHZ_8:
            BCSCTL1 = CALBC1_8MHZ;
            DCOCTL  = CALDCO_8MHZ;
            break;
        case MHZ_12:
            BCSCTL1 = CALBC1_12MHZ;
            DCOCTL  = CALDCO_12MHZ;
            break;
        case MHZ_16:
            BCSCTL1 = CALBC1_16MHZ;
            DCOCTL  = CALDCO_16MHZ;
            break;
        //tocando registros DCOCTL (DCOx, MODx) y BCSCTL1 (RSELx), se
        //deberia poder obtener otras frecuencias. Los valores de
        //fabrica de 1 a 16MHz dejan DCOx=0.
        }
}

/******************************************************************************/

void setRTI(uint8_t rti_period_in_us, uint8_t rti_timer)
{
    //Si elijo MS_0064, MS_05, MS_8, MS_32, se usa el wdt. Por prolijidad, uno deberia elegir el RTI_WDT.
    //Si elijo alguna de las otras, se debe elegir entre Timer0_A y Timer1_A con RTI_TIMER0, RTI_TIMER1.
    //Si se hacen cosas raras como elegir MS_0064 y RTI_TIMER0, se pueden obtener comportamientos raros.

    uint16_t aux[3];

    switch(rti_period_in_us)
    {
    //por Watchdog
    case MS_0064:
        WDTCTL = WDT_MDLY_0_064; //frena el WDT, selecciona modo interrupt, lo pone en 0, elige interrupts cada 0.064ms.
        IE1 |=  WDTIE; // Enable WDT (periodic) interrupts;
        break;
    case MS_05:
        WDTCTL = WDT_MDLY_0_5; //idem, elige interrupts cada 0.5ms.
        IE1 |=  WDTIE;
        break;
    case MS_8:
        WDTCTL = WDT_MDLY_8; //idem, elige interrupts cada 0.8ms.
        IE1 |=  WDTIE;
        break;
    case MS_32:
        WDTCTL = WDT_MDLY_32; //idem, elige interrupts cada 0.32ms.
        IE1 |=  WDTIE;
        break;
    //por Timer0_A o Timer1_A
    case MS_1:
        aux[0] = TASSEL_2 + ID_0 + MC_1; //selecciona SMCLK, lo divide por 1, count up mode, hasta TA1CCR0
        aux[1] = CCR0_1MS; //cuenta de 0 a 999: para 1MHz, tarda 1ms entre interrupciones
        aux[2] = CCIE; //(periodic) interrupt enable
        break;
    case MS_2:
        aux[0] = TASSEL_2 + ID_0 + MC_1;
        aux[1] = CCR0_2MS; //cuenta de 0 a 1999: para 1MHz, tarda 2ms
        aux[2] = CCIE;
        break;
    case MS_5:
        aux[0] = TASSEL_2 + ID_0 + MC_1;
        aux[1] = CCR0_5MS; //tarda 5ms
        aux[2] = CCIE;
        break;
    case MS_10:
        aux[0] = TASSEL_2 + ID_0 + MC_1;
        aux[1] = CCR0_10MS; //tarda 10ms
        aux[2] = CCIE;
        break;
    case MS_50:
        aux[0] = TASSEL_2 + ID_0 + MC_1;
        aux[1] = CCR0_50MS; //tarda 50ms
        aux[2] = CCIE;
        break;
    }

    if(rti_timer==RTI_TIMER0) //por Timer0_A
    {
        TA0CTL = aux[0];
        TA0CCR0 = aux[1];
        TA0CCTL0 |= aux[2];
    }
    else if(rti_timer==RTI_TIMER1) //por Timer1_A
    {
        TA1CTL = aux[0];
        TA1CCR0 = aux[1];
        TA1CCTL0 |= aux[2];
    }
}

/******************************************************************************/
