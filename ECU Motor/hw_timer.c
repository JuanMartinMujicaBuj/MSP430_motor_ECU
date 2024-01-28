/***************************************************************************//**
  @file     hw_timer.c
  @brief    Hardware timer services.
  @authors  Jacoby, Mujica Buj, Olivera, Torres, Zannier Diaz
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include "hw_timer.h"
#include "gpio.h"

/*******************************************************************************
 * ENUMERATIONS, STRUCTURES AND TYPEDEFS
 ******************************************************************************/
typedef void (*callback_t)(void);

/*******************************************************************************
 * STATIC VARIABLES AND CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/
static callback_t saved_timer0_ccr0_callback; //callbacks de los flags de ccr0 y ccr1
static callback_t saved_timer0_ccr1_callback;
static callback_t saved_timer1_ccr0_callback;
static callback_t saved_timer1_ccr1_callback;

static uint16_t dummy; //para leer TA0IV y borrar la pending interrupt flag del
                       //CCR1 (o TAIFG llegado el caso)

/*******************************************************************************
* GLOBAL FUNCTION DEFINITIONS
*******************************************************************************/
//Inicializar Timer_A: elegir clocks.
//timer_num: Timer0_A-->0, Timer1_A-->1
void hwTimerInit(uint8_t timer_num)
{
    //Chequeo de que no se haya inicializado antes
    static uint8_t yaInit[2] = {0,0};
    if (yaInit[timer_num])
        return;
    yaInit[timer_num] = 1;

    //Seteo registros
    switch(timer_num)
    {
    case 0:
        TA0CCR0 = 0; //Inicialiazación: parada del Timer
        TA0CTL = TASSEL_2 + ID_0; //Selecciona SMCLK, SMCLK/1
        break;
    case 1:
        TA1CCR0 = 0;
        TA1CTL = TASSEL_2 + ID_0;
        break;
    }
}

/******************************************************************************/

//Iniciar timer.
void hwSetTimer(uint8_t timer_num, uint8_t timer_mode, uint8_t count_mode, uint8_t capture_mode, uint8_t out_mode, uint8_t pin, uint16_t ccr0, uint16_t ccr1, void (*callback_ccr0)(void), void (*callback_ccr1)(void))
{
    uint8_t aux;
    uint16_t aux2;

    switch(timer_num)
    {
    case 0:
        switch(timer_mode)
        {
//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//
        case INPUT_CAPTURE_HWT: //TIMER0_A INPUT CAPTURE
            //set count mode and enable interrupts for overflow management.
            TA0CTL &= ~MC_3; //borrar MC
            switch(count_mode)
            {
            case UP_MODE_HWT:   //el uso de up mode y un pin que use el ccr0 para input capture es incorrecto
                TA0CTL |= MC_1; //y puede dar comportamientos extraños/fallidos
                TA0CCTL0 |= CCIE; //el uso de up mode presupone que uso ccr0 callback para manejo de overflows.
                break;
            case CONT_MODE_HWT: //en modo continuo, habilito interrupts por overflow (que se manejan con ccr1 callback).
                TA0CTL |= MC_2;
                TA0CTL |= TAIE; //el interrupt flag se borra con un "dummy=TAIV;" dentro de la interrupcion
                break;
            case UP_DOWN_MODE_HWT: //no tiene mucho sentido pero la opción está.
                TA0CTL |= MC_3;
                TA0CCTL0 |= CCIE;
                break;
            }

            //set capture mode
            switch(capture_mode)
            {
            case NO_CAPTURE:
                aux2=0;
                break;
            case CAPTURE_ON_RISING_EDGE:
                aux2=CM0;
                break;
            case CAPTURE_ON_FALLING_EDGE:
                aux2=CM1;
                break;
            case CAPTURE_ON_BOTH_EDGES:
                aux2=CM0+CM1;
                break;
            }

            //find out which CCRx and CCIxx to use as input capture according to the pin selected, and set input capture mode.
            switch(pin) //Escribo solo aquellos pines habilitados para input capture con Timer0_A
            {
            case P1_1_HWT: //CCI0A (usar con continuous mode. Manejo de overflows con ccr1 callback)
                TA0CCTL0 |= CAP + aux2 + SCS + CCIE; //ccr0 capture mode: no capture/rising/falling/both, sync capture, interrupt enable
                gpioMode(PORTNUM2PIN(1,1), INPUT_PULLDOWN);
                P1SEL |= BIT1;
                P1SEL2 &= ~BIT1;
                break;
            case P1_2_HWT: //CCI1A (usar con up mode. Manejo de overflows con ccr0 callback).
                TA0CCTL1 |= CAP + aux2 + SCS + CCIE; //ccr1 capture mode: no capture/rising/falling/both, sync capture, interrupt enable
                gpioMode(PORTNUM2PIN(1,2), INPUT_PULLDOWN);
                P1SEL |= BIT2;
                P1SEL2 &= ~BIT2;
                break;
            }

            //Set CCR0
            TA0CCR0 = ccr0;

            //save function callbacks
            saved_timer0_ccr0_callback = callback_ccr0;
            saved_timer0_ccr1_callback = callback_ccr1;
            break;
//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//
        case OUTPUT_COMPARE_HWT: //TIMER0_A OUTPUT COMPARE
            //set count mode
            TA0CTL &= ~MC_3; //borrar MC
            switch(count_mode)
            {
            case UP_MODE_HWT:
                TA0CTL |= MC_1;
                break;
            case CONT_MODE_HWT:
                TA0CTL |= MC_2;
                break;
            case UP_DOWN_MODE_HWT:
                TA0CTL |= MC_3;
                break;
            }

            //set out mode
            TA0CCTL0 &= ~OUTMOD_7; //borrar OUTMOD
            TA0CCTL1 &= ~OUTMOD_7; //como no se a priori que pin uso, toco los 3 puertos
            TA0CCTL2 &= ~OUTMOD_7; //NO programo acciones avanzadas como hacer un set-reset en un pin de Out1, y un toggle-set en uno de Out2.
            switch(out_mode)       //si entendí bien el módulo, como no configuro ccr2, los de Out2 no deberian andar.
            {
            case OUTPUT_MODE_HWT:
                aux = OUTMOD_0;
                break;
            case SET_MODE_HWT:
                aux = OUTMOD_1;
                break;
            case TOGGLE_RESET_MODE_HWT:
                aux = OUTMOD_2;
                break;
            case SET_RESET_MODE_HWT:
                aux = OUTMOD_3;
                break;
            case TOGGLE_MODE_HWT:
                aux = OUTMOD_4;
                break;
            case RESET_MODE_HWT:
                aux = OUTMOD_5;
                break;
            case TOGGLE_SET_MODE_HWT:
                aux = OUTMOD_6;
                break;
            case RESET_SET_MODE_HWT:
                aux = OUTMOD_7;
                break;
            }
            TA0CCTL0 |= aux;
            TA0CCTL1 |= aux;
            TA0CCTL2 |= aux;

            //Set pin (los Out0 se comandan por TAxCCTL0 y no admiten modos 2,3,6,7. Los Out1 se comandan por TAxCCTL1, Out2 por TAxCCTL2)
            if(out_mode != OUTPUT_MODE_HWT)
            {
                switch(pin) //Escribo solo aquellos pines habilitados para outmod con Timer0_A
                {
                case P1_1_HWT:
                    gpioMode(PORTNUM2PIN(1,1), OUTPUT);
                    P1SEL |= BIT1;
                    P1SEL2 &= ~BIT1;
                    break;
                case P1_2_HWT:
                    gpioMode(PORTNUM2PIN(1,2), OUTPUT);
                    P1SEL |= BIT2;
                    P1SEL2 &= ~BIT2;
                    break;
                case P1_5_HWT:
                    gpioMode(PORTNUM2PIN(1,5), OUTPUT);
                    P1SEL |= BIT5;
                    P1SEL2 &= ~BIT5;
                    break;
                case P1_6_HWT:
                    gpioMode(PORTNUM2PIN(1,6), OUTPUT);
                    P1SEL |= BIT6;
                    P1SEL2 &= ~BIT6;
                    break;
                case P2_6_HWT:
                    gpioMode(PORTNUM2PIN(2,6), OUTPUT);
                    P2SEL |= BIT6;
                    P2SEL2 &= ~BIT6;
                    P2SEL &= ~BIT7; //rareza del pin 2.6, ver datasheet pag 53/76
                    P2SEL2 &= ~BIT7;
                    break;
                }
            }

            //Set CCR0, CCR1 (no uso CCR2)
            TA0CCR0 = ccr0;
            TA0CCR1 = ccr1;

            //save function callbacks
            saved_timer0_ccr0_callback = callback_ccr0;
            saved_timer0_ccr1_callback = callback_ccr1;

            //enable ccr0 and ccr1 interrupts
            //TA0CTL |= TAIE; //habilitacion de interrupciones para TAIFG (overflow). Como no la uso, no la activo.
            if(*callback_ccr0) //si tengo una funcion de interrupcion para ccr0 distinta de NULL
            {
                TA0CCTL0 |= CCIE; //Habilitación de interrupciones para CCR0.
            }
            if(*callback_ccr1) //si tengo una funcion de interrupcion para ccr1 distinta de NULL
            {
                TA0CCTL1 |= CCIE; //Habilitación de interrupciones para CCR1.
            }
            break;
        }
        break;
//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//
    case 1:
        switch(timer_mode)
        {
        case INPUT_CAPTURE_HWT: //TIMER1_A INPUT CAPTURE
            //set count mode and enable interrupts for overflow management.
            TA1CTL &= ~MC_3; //borrar MC
            switch(count_mode)
            {
            case UP_MODE_HWT:   //el uso de up mode y un pin que use el ccr0 para input capture es incorrecto
                TA1CTL |= MC_1; //y puede dar comportamientos extraños/fallidos
                TA1CCTL0 |= CCIE; //el uso de up mode presupone que uso ccr0 callback para manejo de overflows.
                break;
            case CONT_MODE_HWT: //en modo continuo, habilito interrupts por overflow (que se manejan con ccr1 callback).
                TA1CTL |= MC_2;
                TA1CTL |= TAIE; //el interrupt flag se borra con un "dummy=TAIV;" dentro de la interrupcion
                break;
            case UP_DOWN_MODE_HWT: //no tiene mucho sentido pero la opción está.
                TA1CTL |= MC_3;
                TA1CCTL0 |= CCIE;
                break;
            }

            //set capture mode
            switch(capture_mode)
            {
            case NO_CAPTURE:
                aux2=0;
                break;
            case CAPTURE_ON_RISING_EDGE:
                aux2=CM0;
                break;
            case CAPTURE_ON_FALLING_EDGE:
                aux2=CM1;
                break;
            case CAPTURE_ON_BOTH_EDGES:
                aux2=CM0+CM1;
                break;
            }

            //find out which CCRx and CCIxx to use as input capture according to the pin selected, and set input capture mode.
            switch(pin) //Escribo solo aquellos pines habilitados para input capture con Timer0_A
            {
            case P2_0_HWT: //CCI0A (usar con continuous mode. Manejo de overflows con ccr1 callback)
                TA1CCTL0 |= CAP + aux2 + SCS + CCIE; //ccr0 capture mode: no capture/rising/falling/both, sync capture, interrupt enable
                gpioMode(PORTNUM2PIN(2,0), INPUT_PULLDOWN);
                P2SEL |= BIT0;
                P2SEL2 &= ~BIT0;
                break;
            case P2_3_HWT: //CCI0B (usar con continuous mode. Manejo de overflows con ccr1 callback)
                TA1CCTL0 |= CAP + aux2 + SCS + CCIE; //ccr0 capture mode: no capture/rising/falling/both, sync capture, interrupt enable
                gpioMode(PORTNUM2PIN(2,3), INPUT_PULLDOWN);
                P2SEL |= BIT3;
                P2SEL2 &= ~BIT3;
                break;
            case P2_1_HWT: //CCI1A (usar con up mode. Manejo de overflows con ccr0 callback).
                TA1CCTL1 |= CAP + aux2 + SCS + CCIE; //ccr1 capture mode: no capture/rising/falling/both, sync capture, interrupt enable
                gpioMode(PORTNUM2PIN(2,1), INPUT_PULLDOWN);
                P2SEL |= BIT1;
                P2SEL2 &= ~BIT1;
                break;
            case P2_2_HWT: //CCI1B (usar con up mode. Manejo de overflows con ccr0 callback).
                TA1CCTL1 |= CAP + aux2 + SCS + CCIE; //ccr1 capture mode: no capture/rising/falling/both, sync capture, interrupt enable
                gpioMode(PORTNUM2PIN(2,2), INPUT_PULLDOWN);
                P2SEL |= BIT2;
                P2SEL2 &= ~BIT2;
                break;
            case P2_4_HWT: //CCI2A (usar con up mode. Manejo de overflows con ccr0 callback)
                TA1CCTL2 |= CAP + aux2 + SCS + CCIE; //ccr2 capture mode: no capture/rising/falling/both, sync capture, interrupt enable
                gpioMode(PORTNUM2PIN(2,4), INPUT_PULLDOWN);
                P2SEL |= BIT4;
                P2SEL2 &= ~BIT4;
                break;
            case P2_5_HWT: //CCI2B (usar con up mode. Manejo de overflows con ccr0 callback).
                TA1CCTL2 |= CAP + aux2 + SCS + CCIE; //ccr2 capture mode: no capture/rising/falling/both, sync capture, interrupt enable
                gpioMode(PORTNUM2PIN(2,5), INPUT_PULLDOWN);
                P2SEL |= BIT5;
                P2SEL2 &= ~BIT5;
                break;
            }

            //Set CCR0
            TA1CCR0 = ccr0;

            //save function callbacks
            saved_timer1_ccr0_callback = callback_ccr0;
            saved_timer1_ccr1_callback = callback_ccr1;
            break;
//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//
        case OUTPUT_COMPARE_HWT: //TIMER1_A OUTPUT COMPARE
            //set count mode
            TA1CTL &= ~MC_3; //borrar MC
            switch(count_mode)
            {
            case UP_MODE_HWT:
                TA1CTL |= MC_1;
                break;
            case CONT_MODE_HWT:
                TA1CTL |= MC_2;
                break;
            case UP_DOWN_MODE_HWT:
                TA1CTL |= MC_3;
                break;
            }

            //set out mode
            TA1CCTL0 &= ~OUTMOD_7; //borrar OUTMOD
            TA1CCTL1 &= ~OUTMOD_7; //como no se a priori que pin uso, toco los 3 puertos
            TA1CCTL2 &= ~OUTMOD_7; //NO programo acciones avanzadas como hacer un set-reset en un pin de Out1, y un toggle-set en uno de Out2.
            switch(out_mode)
            {
            case OUTPUT_MODE_HWT:
                aux = OUTMOD_0;
                break;
            case SET_MODE_HWT:
                aux = OUTMOD_1;
                break;
            case TOGGLE_RESET_MODE_HWT:
                aux = OUTMOD_2;
                break;
            case SET_RESET_MODE_HWT:
                aux = OUTMOD_3;
                break;
            case TOGGLE_MODE_HWT:
                aux = OUTMOD_4;
                break;
            case RESET_MODE_HWT:
                aux = OUTMOD_5;
                break;
            case TOGGLE_SET_MODE_HWT:
                aux = OUTMOD_6;
                break;
            case RESET_SET_MODE_HWT:
                aux = OUTMOD_7;
                break;
            }
            TA1CCTL0 |= aux;
            TA1CCTL1 |= aux;
            TA1CCTL2 |= aux;

            //Set pin (los Out0 se comandan por TAxCCTL0 y no admiten modos 2,3,6,7. Los Out1 se comandan por TAxCCTL1, Out2 por TAxCCTL2)
            if(out_mode != OUTPUT_MODE_HWT)
            {
                switch(pin) //Escribo solo aquellos pines habilitados para outmod con Timer1_A
                {
                case P2_0_HWT:
                    gpioMode(PORTNUM2PIN(2,0), OUTPUT);
                    P2SEL |= BIT0;
                    P2SEL2 &= ~BIT0;
                    break;
                case P2_1_HWT:
                    gpioMode(PORTNUM2PIN(2,1), OUTPUT);
                    P2SEL |= BIT1;
                    P2SEL2 &= ~BIT1;
                    break;
                case P2_2_HWT:
                    gpioMode(PORTNUM2PIN(2,2), OUTPUT);
                    P2SEL |= BIT2;
                    P2SEL2 &= ~BIT2;
                    break;
                case P2_3_HWT:
                    gpioMode(PORTNUM2PIN(2,3), OUTPUT);
                    P2SEL |= BIT3;
                    P2SEL2 &= ~BIT3;
                    break;
                case P2_4_HWT:
                    gpioMode(PORTNUM2PIN(2,4), OUTPUT);
                    P2SEL |= BIT4;
                    P2SEL2 &= ~BIT4;
                    break;
                case P2_5_HWT:
                    gpioMode(PORTNUM2PIN(2,5), OUTPUT);
                    P2SEL |= BIT5;
                    P2SEL2 &= ~BIT5;
                    break;
                }
            }

            //Set CCR0, CCR1 (no uso CCR2)
            TA1CCR0 = ccr0;
            TA1CCR1 = ccr1;

            //save function callbacks
            saved_timer1_ccr0_callback = callback_ccr0;
            saved_timer1_ccr1_callback = callback_ccr1;

            //enable ccr0 and ccr1 interrupts
            //TA1CTL |= TAIE; //habilitacion de interrupciones para TAIFG (overflow). Como no la uso, no la activo.
            if(*callback_ccr0) //si tengo una funcion de interrupcion para ccr0 distinta de NULL
            {
                TA1CCTL0 |= CCIE; //Habilitación de interrupciones para CCR0.
            }
            if(*callback_ccr1) //si tengo una funcion de interrupcion para ccr1 distinta de NULL
            {
                TA1CCTL1 |= CCIE; //Habilitación de interrupciones para CCR1.
            }
            break;
        }
        break;
    }
}

/******************************************************************************/

//Devuelve ticks del free running counter (aka TAR), para lo que sea que se quiera.
uint16_t hwTimerGetTicks(uint8_t timer_num)
{
    if(!timer_num) //Timer0_A
    {
        return TA0R;
    }
    else //Timer1_A
    {
        return TA1R;
    }

}

/******************************************************************************/

//incrementar ccr0 en la cantidad deseada. Util para armar una posible funcion de callback_ccr0.
void hwTimerIncCCR0(uint16_t num, uint8_t timer_num)
{
    if(!timer_num) //Timer0_A
        {
            TA0CCR0 += num;
        }
        else //Timer1_A
        {
            TA1CCR0 += num;
        }
}

/******************************************************************************/

//incrementar ccr1 en la cantidad deseada. Util para armar una posible funcion de callback_ccr1.
void hwTimerIncCCR1(uint16_t num, uint8_t timer_num)
{
    if(!timer_num) //Timer0_A
        {
            TA0CCR1 += num;
        }
        else //Timer1_A
        {
            TA1CCR1 += num;
        }
}

/******************************************************************************/

//Darle a ccr0 el valor deseado. Util para armar una posible funcion de callback_ccr0.
void hwTimerSetCCR0(uint16_t num, uint8_t timer_num)
{
    if(!timer_num) //Timer0_A
        {
            TA0CCR0 = num;
        }
        else //Timer1_A
        {
            TA1CCR0 = num;
        }
}

/******************************************************************************/

//Darle a ccr1 el valor deseado. Util para armar una posible funcion de callback_ccr0.
void hwTimerSetCCR1(uint16_t num, uint8_t timer_num)
{
    if(!timer_num) //Timer0_A
        {
            TA0CCR1 = num;
        }
        else //Timer1_A
        {
            TA1CCR1 = num;
        }
}

/******************************************************************************/
//Leer valor del CCR0 (util para input capture)
uint16_t hwTimerGetCCR0(uint8_t timer_num)
{
    if(!timer_num) //Timer0_A
        {
            return TA0CCR0;
        }
        else //Timer1_A
        {
            return TA1CCR0;
        }
}
/******************************************************************************/
//Leer valor del CCR1 (util para input capture)
uint16_t hwTimerGetCCR1(uint8_t timer_num)
{
    if(!timer_num) //Timer0_A
        {
            return TA0CCR1;
        }
        else //Timer1_A
        {
            return TA1CCR1;
        }
}

/******************************************************************************/
//Leer valor del CCR2 (util para input capture)
uint16_t hwTimerGetCCR2(uint8_t timer_num)
{
    if(!timer_num) //Timer0_A
        {
            return TA0CCR2;
        }
        else //Timer1_A
        {
            return TA1CCR2;
        }
}

/******************************************************************************/
//Frenar el timer. Count mode: stop mode
void hwStopTimer(uint8_t timer_num)
{
    if(!timer_num) //Timer0_A
        {
            TA0CTL &= ~MC_3; //borrar MC
            TA0CTL |= MC_0; //stop mode
        }
        else //Timer1_A
        {
            TA1CTL &= ~MC_3; //borrar MC
            TA1CTL |= MC_0; //stop mode
        }
}

/******************************************************************************/
/******************************************************************************/
#ifdef TIMER0_USED_BY_HWT
#pragma vector=TIMER0_A0_VECTOR        //Interrupt Service Routine (ISR) for CCR0 (only)
__interrupt void isr_Timer0_CCR0(void) //The TACCR0 CCIFG flag is automatically reset when the TACCR0 interrupt request is serviced (p367/344)
{
    (*saved_timer0_ccr0_callback)(); //call ccr0 callback function
}

/******************************************************************************/

#pragma vector=TIMER0_A1_VECTOR        //Interrupt Service Routine (ISR) for overflow and CCR<1-n> TimerA0
__interrupt void isr_Timer0_CCR1(void) //Any access, read or write, of the TAIV register automatically
{                                      //resets the highest pending interrupt flag [of this register].
    dummy=TA0IV; // Clear Interrupt flag (whether it is TACCR1 CCIFG, TACCR2 CCIFG or TAIFG)
    (*saved_timer0_ccr1_callback)(); //call ccr1 callback function
}
#endif

/******************************************************************************/
#ifdef TIMER1_USED_BY_HWT
#pragma vector=TIMER1_A0_VECTOR
__interrupt void isr_Timer1_CCR0(void)
{
    (*saved_timer1_ccr0_callback)();
}

/******************************************************************************/
#pragma vector=TIMER1_A1_VECTOR
__interrupt void isr_Timer1_CCR1(void)
{
    dummy=TA1IV;
    (*saved_timer1_ccr1_callback)();
}
#endif
