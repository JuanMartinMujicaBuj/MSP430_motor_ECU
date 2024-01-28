/***************************************************************************//**
  @file     comms.c
  @brief    Communication through UART and SPI-CAN.
  @authors  Mujica Buj
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include "mcp2515.h"
#include "uart.h"
#include "gpio.h"

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

/*******************************************************************************
 * STATIC VARIABLES AND CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/
#define CAN_INT PORTNUM2PIN(1,4)
static can_t can_a_enviar; //can frame a la que escribo para enviar
static can_t can_a_recibir; //can frame de la que leo lo que recibo
static uint8_t can_recibido_flag; //flag de que recibi un can frame
//el driver de uart tiene dentro suyo una uart_recibido_flag, a la que accedo con
//uartStatus() ("uart" en minúsculas)

/*******************************************************************************
* GLOBAL FUNCTION DEFINITIONS
*******************************************************************************/
//inicializar UART y SPI-CAN
void commsInit(uint8_t uart_msg_length)
{
    if(uart_msg_length) //si ingreso un numero distinto de 0
    {
        uartInit(uart_msg_length); //inicializar uart
    }

    MCP2515_init(); //inicializar SPI y CAN
    MCP2515_CanVariable_init(&can_a_enviar); //inicializar variable can: por defecto, largo: 8 bytes, todos valen 0.
    MCP2515_CanVariable_init(&can_a_recibir);
    gpioMode(CAN_INT,INPUT_PULLUP); //pin para interrupt de CAN con input pullup (normal alto/activo bajo)
    P1IE |= BIT4; //interrupt enable for pin 4, port 1
    P1IES |= BIT4; //interrupt on falling edge
    P1IFG &= ~BIT4; //reset pending interrupt flag
}

/******************************************************************************/
//Enviar mensaje por uart
void UARTSend(uint8_t string[])
{
    uartWriteString(string); //Carga el string a enviar en el buffer del driver de uart.
    uartSend(); //Habilita interrupts dedicadas para enviar bits. Estos se van cargando
                //desde el buffer del driver de uart, hacia el TXBUF. Cuando no queda
                //nada por enviar, se deshabilitan dichas interrupts dedicadas.
}

/******************************************************************************/
//Enviar mensaje por CAN
void CANSend(uint8_t largo, uint8_t bytes[])
{
    uint8_t i;
    can_a_enviar.dlc = largo; //ajusto el largo del mensaje
    for(i=0;i<largo;++i)
    {
        can_a_enviar.data[i]=bytes[i]; //cargo el mensaje en mi can frame
    }
    MCP2515_can_tx0(&can_a_enviar); //por SPI, doy la instruccion al mcp2515+tja1050
                                    //de construir y enviar el mensaje.
}

/******************************************************************************/
//Recibir mensaje por uart
void UARTReceive(uint8_t *p2char)
{
    uartRead(p2char);
    //A medida que llegan bits al RXBUF, por interrupts dedicadas el driver de uart
    //las va copiando al receive buffer del driver de uart.
    //Con uartRead, se copia lo que se haya acumulado en el receive buffer, a donde
    //apunte el p2char.
}

/******************************************************************************/
//Recibir mensaje por CAN
void CANReceive(uint8_t largo, uint8_t *p2char)
{
    uint8_t i;
    for(i=0;i<largo;++i)
    {
        *(p2char+i)=can_a_recibir.data[i]; //copio el mensaje que tengo en el receive buffer,
    }                                      //a donde sea que aputen el p2char
}

/******************************************************************************/
//ver si hay mensajes nuevos de uart
uint8_t UARTStatus(void)
{
    return uartStatus(); //se encarga el driver de uart de avisarme si recibi un msj o no.
}

/******************************************************************************/
//ver si hay mensajes nuevos de CAN
uint8_t CANStatus(void)
{
    static uint8_t aux; //variabel auxiliar que vive dentro de la funcion
    aux=can_recibido_flag; //copio flag
    if(aux)
    {
        can_recibido_flag=0; //borro flag
    }
    return aux; //devuelvo copia del flag
}

/******************************************************************************/
//Armo mensaje a enviar con variables de 1 byte (uint8_t, int8_t)
void CANBuildMsg8(uint8_t can_to_send[], uint8_t var8, uint8_t pos)
{
    can_to_send[pos] = var8;
}

/******************************************************************************/
//Armo mensaje a enviar con variables de 2 bytes (uint16_t, int16_t)
void CANBuildMsg16(uint8_t can_to_send[], uint16_t var16, uint8_t pos)
{
    can_to_send[pos++]=(uint8_t)(var16>>8); //separo la variable (u)int16 en sus 2 bytes y
    var16 <<= 8;                            //los guardo por separado en el array de chars.
    can_to_send[pos]=(uint8_t)(var16>>8);
}

/******************************************************************************/
//Armo mensaje a enviar con variables de 4 bytes (uint32_t, int32_t)
void CANBuildMsg32(uint8_t can_to_send[], uint32_t var32, uint8_t pos)
{
    uint8_t i;                               //separo la variable (u)int32 en sus 4 bytes y
    can_to_send[pos++]=(uint8_t)(var32>>24); //los guardo por separado en el array de chars.
    for(i=0;i<3;i++)
    {
        var32 <<= 8;
        can_to_send[pos++]=(uint8_t)(var32>>24);
    }
}

/******************************************************************************/
//Armo mensaje a enviar con variable de 8 bytes (uint64_t, int64_t)
void CANBuildMsg64(uint8_t can_to_send[], uint64_t var64, uint8_t pos)
{
    uint8_t i;                               //separo la variable (u)int64 en sus 8 bytes y
    can_to_send[pos++]=(uint8_t)(var64>>56); //los guardo por separado en el array de chars.
    for(i=0;i<7;i++)
    {
        var64 <<= 8;
        can_to_send[pos++]=(uint8_t)(var64>>56);
    }
    //Para este caso, como el can_to_send tiene un largo de 8, el unico valor valido de pos es 0, asi escribo del byte 0 al 8.
}

/******************************************************************************/
//Extraer variable de 1 byte de largo de un mensaje de CAN
uint8_t CANUnbuildMsg8(uint8_t can_received[], uint8_t pos)
{
    return can_received[pos];
}

/******************************************************************************/
//Extraer variable de 2 bytes de largo de un mensaje de CAN
uint16_t CANUnbuildMsg16(uint8_t can_received[], uint8_t pos)
{
    uint16_t var16=((uint16_t)can_received[pos]<<8)+(uint16_t)can_received[pos+1];
    return var16;
}

/******************************************************************************/
//Extraer variable de 4 bytes de largo de un mensaje de CAN
uint32_t CANUnbuildMsg32(uint8_t can_received[], uint8_t pos)
{
    uint32_t var32=0;
    int8_t i;
    for(i=3;i>=0;i--)
    {
        var32 += (uint32_t)can_received[pos++]<<(8*i);
    }
    return var32;
}

/******************************************************************************/
//Extraer variable de 8 bytes de largo de un mensaje de CAN
uint64_t CANUnbuildMsg64(uint8_t can_received[], uint8_t pos)
{
    uint64_t var64=0;
    int8_t i;
    for(i=7;i>=0;i--)
    {
        var64 += (uint64_t)can_received[pos++]<<(8*i);
    }
    return var64;
}

/*******************************************************************************
 * INTERRUPT SERVICE ROUTINE DEFINITION
 ******************************************************************************/
//Interrupt dedicada del CAN por medio del pin INT (que colocamos en el P1.4 del MSP)
// Port 1: Interrupt-Service-Routine
#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{
    if(P1IFG & BIT4) //si la interrupt la activo el bit 4 (INT del CAN)
    {
        P1IFG &= ~BIT4; //bajar interrupt flag
        MCP2515_can_rx0(&can_a_recibir); //cargar lo recibido en el can_a_recibir
        can_recibido_flag=1; //levanto flag de msj recibido
    }
}

/******************************************************************************/

//Pensar si hay riesgo de que se rompa algo si quiero leer un dato cuando estoy
//recibiendo datos por uart, o si quiero escribir un dato cuando estoy enviando
//datos por uart:

//puede que en uartRead(), vea vacia la posicion i, entonces me salteo el while,
//justo despues llega un dato a la posicion i, pero como ya saltee el while,
//reseteo igual el i_receive y no llegue a copiar el dato nuevo, que queda suelto
//en la posicion i. Solucion: deshabilitar interrupts cuando leo? pero capaz pierdo
//datos en ese tiempo. Puede hacer mas mal que bien...

//con transmit creo que no pasa porque el interrupt tiene prioridad y si justo escribo
//un dato nuevo cuando estoy en el medio del interrupt, primero se resetea i_transmit,
//y despues se escribe el dato nuevo en la posicion i=0.

//SOLUCION:
//acivar periodicamente un comsFlag: cuando la leo, uso readString(), writeString(), send().
//Y ni bien envío, que el otro lado me envíe a mí. Entonces sé que para la proxima vez que
//lea, ya voy a haber terminado de recibir.
//Para nuestro tp, el que coordina los envios es el ECU Tablero: envia a la PC y la PC
//entonces empieza a enviar; envía a la ECU Motor y la ECU Motor entonces empieza a enviar.
