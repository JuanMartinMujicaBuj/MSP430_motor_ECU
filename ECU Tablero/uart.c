/***************************************************************************//**
  @file     uart.c
  @brief    UART serial communication services.
  @authors  Jacoby, Mujica Buj
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include "uart.h"

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/
#define MAX_CHARS 49 //maxima cantidad de caracteres almacenables en el buffer
                     //para ser transmitidos de un saque.

/*******************************************************************************
 * STATIC VARIABLES AND CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/
static uint8_t transmit_buffer[MAX_CHARS+1];
//buffer por software para guardar (con uartPutChar()) el texto a transmitir
//(con uartSend()). La ultima posicion siempre se deja en 0.

static uint8_t receive_buffer[MAX_CHARS+1];
//buffer por software para guardar (con las interrupts dedicadas en RXIFG)
//el texto a leer (con uartRead()). La ultima posicion siempre se deja en 0.

static uint8_t i_transmit; //indice del transmit buffer
static uint8_t i_receive; //indice del receive buffer
static uint8_t i_TX; //indice para transmitir caracteres con uartPutChar()

static uint8_t n_chars; //cantidad de chars que tiene el msj que espero recibir
static uint8_t uart_recibido_flag; //flag que se activa al acumular N chars en el buffer

/*******************************************************************************
 * PRIVATE FUNCTION PROTOTYPES
 ******************************************************************************/
void uartPutChar(uint8_t c); //envia un char: lo pone en el TXBUF
uint8_t uartGetChar(void); //recibe un char: lo lee del RXBUF
void uart_transmit_rti(void); //rutina de interrupcion de uart transmit
void uart_receive_rti(void); //rutina de interrupcion de uart receive

/*******************************************************************************
* GLOBAL FUNCTION DEFINITIONS
*******************************************************************************/
void uartInit(uint8_t n_char)
{
    //Los pines para UART son P1.1 y P1.2.
    //Estos pines por defecto estan en modo gpio (de entre todos los modos que se pueden multiplexar).
    //Debo cambiar su P1SEL, P1SEL2 para que esten en modo RXD, TXD
    P1SEL = BIT1 | BIT2 ; //Pongo ambos bits en 1, en P1SEL y en P1SEL2
    P1SEL2 = BIT1 | BIT2;

    //UCA0CTL0 = 0;
    //Parity disabled (UCPEN=0)
    //Odd parity (UCPAR=0)
    //LSB first (UCMSB=0)
    //8-bit data (UC7BIT=0)
    //one stop bit (UCSPB=0)
    //UART mode (UCMODEx=00)
    //Asynchronous mode (UCSYNC=0)

    UCA0CTL1 = UCSWRST;
    //software reset enabled (UCA0 placed in Reset to be configured). USCI logic held in reset state. (UCSWRST=1)
    UCA0CTL1 = UCSSEL_2;
    //clock: SMCLK (UCSSELx=10)
    //Receive erroneous-character interrupt-enable: Erroneous characters rejected and UCAxRXIFG is not set (UCRXEIE=0)
    //Receive break character interrupt-enable: Received break characters do not set UCAxRXIFG. (UCBRKIE=0)
    //Not dormant. All received characters will set UCAxRXIFG. (UCDORM=0)
    //Next frame transmitted is data (not an address) (UCTXADDR=0)
    //Next frame transmitted is not a break (UCTXBRK=0)

    UCA0BR0 = 104; // Baud rate of 9600 for BRCLK frequency of 1MHz
    UCA0BR1 = 0; // Baud rate of 9600 for BRCLK frequency of 1MHz
    UCA0MCTL = UCBRS0; // // Baud rate of 9600 for BRCLK frequency of 1MHz. Modulation UCBRSx = 1
    //Oversampling mode disabled (UCOS16=0)

    UCA0CTL1 &= ~UCSWRST; //software reset disabled. USCI reset released for operation. (UCSWRST=0)

    IE2 |= UCA0RXIE; //receive interrupt enabled (siempre puede recibir)
    //El transmit interrupt lo habilito solo cuando quiero enviar, con uartSend()

    //UCA0RXBUF, UCA0TXBUF buffers de receive y transmit data
    //IE2, IFG2 interrupt enable e interrupt flag (activar, leer)
    //UCA0STAT status register (no nos interesa por ahora)
    //UCA0IRTCTL, UCA0IRRCTL para IrDA (no nos interesa)
    //UCA0ABCTL LIN control (no nos interesa)
    //no tenemos en el msp430G2553 USCI_A1 (solo USCI_A0). Y el USCI_B0 se puede usar para SPI, I2C (no UART)

    n_chars = n_char; //copio el valor a un buffer del driver. Cuando se acumulen n_chars en el
                      //receive buffer, levanto un flag. Lo bajo cuando pida uartStatus()
}

/******************************************************************************/

void uartWriteChar(uint8_t c)
{
    transmit_buffer[i_transmit++] = c;
    if(i_transmit==MAX_CHARS)
    {
        i_transmit=0;
    }
    //Guarda c en la proxima posicion del transmit_buffer, luego se suma 1 al indice.
    //Si el indice se pasa de MAX_CHARS, vuelve a 0.
}

/******************************************************************************/

void uartWriteString(uint8_t string[])
{ //Escribe el string en el transmit_buffer. Dicho string debe tener el terminador \0.
  //Esta función NO copia dicho \0.
    uint8_t i=0;
    while(string[i])
    {
        transmit_buffer[i_transmit++] = string[i++];
        if(i_transmit==MAX_CHARS)
        {
            i_transmit=0;
        }
    }
    //guarda el array de chars a partir de la proxima posicion del transmit_buffer.
    //Si i_transmit se pasa de MAX_CHARS, vuelve a 0.
}

/******************************************************************************/

void uartSend(void)
{
    IE2 |= UCA0TXIE; //transmit interrupt enabled para mandar todos los chars acumulados
                     //en el transmit_buffer. Cuando termine, se deberá desactivar.
}

/******************************************************************************/

uint8_t uartReadChar(void)
{ //i_receive siempre apunta a la primer posicion vacia. Luego, me interesa leer
  //la posicion anterior (la ultima llena).
    if(i_receive)
    {
        return receive_buffer[i_receive-1];
    }
    else //si i_receive==0
    {
        return 0;
    }
}

/******************************************************************************/

void uartRead(uint8_t *p2char)
{ //Copia los contenidos del receive_buffer en el array de chars al que apunte p2char.
  //Borra los chars del receive_buffer a medida que los va leyendo.
  //Frena donde encuentra un \0, el cual NO se copia.
    uint8_t i=0;
    while(receive_buffer[i])
    {
        *(p2char+i) = receive_buffer[i];
        receive_buffer[i++] = 0;
    }
    i_receive=0; //resetea el indice que va llenando el receive_buffer
}

/******************************************************************************/
uint8_t uartStatus(void)
{
    static uint8_t aux; //variable auxiliar que vive dentro de la funcion
    aux=uart_recibido_flag; //copio flag
    if(aux)
    {
        uart_recibido_flag=0; //borro flag
    }
    return aux; //devuelvo copia del flag
}
/*******************************************************************************
 * PRIVATE FUNCTION DEFINITIONS
 ******************************************************************************/
void uartPutChar(uint8_t c)
{
    UCA0TXBUF = c;
}

/******************************************************************************/

uint8_t uartGetChar(void)
{ //"leer el rxbuf", se resetea el rxbuf y su interrupt flag
    return UCA0RXBUF;
}

/******************************************************************************/

void uart_transmit_rti(void)
{ //transmite todos los valores acumulados en el transmit_buffer
    if(transmit_buffer[i_TX]) //si transmit_buf[i] es un valor a enviar
    {
        uartPutChar(transmit_buffer[i_TX]); //se envia
        transmit_buffer[i_TX++]=0; //y se borra del buffer
    }
    else //si ya no hay nada mas a enviar
    {
        i_transmit=0; //el transmit_buf esta vacio: se resetea el indice que lo llena
        i_TX=0; //se resetea el indice que lo envia y borra
        IE2 &= ~UCA0TXIE; //se deshabilita el transmit interrupt
    }
}

/******************************************************************************/

void uart_receive_rti(void)
{ //recibe de a un valor y lo acumula en el receive_buffer
    receive_buffer[i_receive++] = uartGetChar();
    if(i_receive==n_chars)
    {                           //si espero mensajes de n_chars y llego al char nro n,
        uart_recibido_flag = 1; //significa que recibí un mensaje completo
    }
    if(i_receive==MAX_CHARS) //si se pasa del tamaño del buffer
    {
        i_receive=0; //vuelve a 0 y sobreescribe lo que habia en el buffer
    }
}

/******************************************************************************/
/******************************************************************************/
//Interrupt Service Routine (ISR) for USCI_A0_TX and USCI_B0_TX
//Transmit interrupt vector for uart (and spi)
#pragma vector = USCIAB0TX_VECTOR
__interrupt void isr_transmit(void)
{
    if(IFG2 & UCA0TXIFG) //si la interrupt la activó UC_A0 (uart):
    {
        uart_transmit_rti();
    }
    //spi: no usa interrupts de TX.
}

/******************************************************************************/

//Interrupt Service Routine (ISR) for USCI_A0_RX and USCI_B0_RX
//Receive interrupt vector for uart, spi, can(?)
#pragma vector = USCIAB0RX_VECTOR
__interrupt void isr_receive(void)
{
    if(IFG2 & UCA0RXIFG) //si la interrupt la activó UC_A0 (uart):
    {
        uart_receive_rti();
    }
}

/******************************************************************************/

