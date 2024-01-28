//CODIGO PARA LA ECU TABLERO
//Maneja un joystick-acelerador, un servo-velocimetro, comunicacion CAN y UART.

//Inicializo drivers, seteo valores iniciales de variables.

//Si no estoy en modoRemoto, leo el valor del joystick.
//Lo envio, junto con las rpm's del motor medidas por el optoacoplador, por UART a la PC.
//Envio valor del joystick y valores del angulo de encendido y apagado de la bujia por CAN a ECU Motor.
//Recibo valor de rpm's que mide el optoacoplador por CAN de ECU Motor.
//Promedio las ultimas 4 mediciones del optoacoplador, las transformo en un angulo y se las escribo al servo.
//(Dicho promedio sera el que envie en el proximo ciclo por UART).
//Recibo por UART de la PC los angulos de encendido y apagado de la bujia, la indicacion de si estoy o no en
//modoRemoto, y un valor de joystick desde la PC que es el que uso si estoy en modo remoto.
//Toggleo un led para indicar que finalice el ciclo.
//Hago t0do esto en ciclos cada poco mas de 100ms, la velocidad la limita el tiempo que necesita la app de la
//PC para graficar la velocidad y el acelerador.

//En paralelo, hago pwm al servo con un timer.

//-------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------
//Drivers
#include "common.h"
#include "board.h"
#include "system.h"
#include "rti.h"
#include "gpio.h"
#include "comms.h"
#include "joystick.h"
#include "servo.h"

//-------------------------------------------------------------------------------------------------------------------
//Defines
#define JOYSTICK2ACEL 1/50 //Escalado del valor del joystick
#define VEL_MAX 216 //Maxima velocidad del motor dc.

//-------------------------------------------------------------------------------------------------------------------
//Prototipos de funciones
void appInit(void); //Inicializar drivers.
void appRun(void);  //Correr logica del programa dentro de un while(TRUE).

void setFlag(void); //Seteo una flag cada 100ms para comenzar un nuevo ciclo.
uint8_t suavizarRpm(uint8_t rpm); //Promedio las ultimas 4 lecturas de rpmMedidas para minimizar oscilaciones del servo.
int8_t getAngulo(uint8_t rpm); //Mapeo de las rpmMedidas, al angulo que le corresponde al servo (velocimetro).

void num2strSigned(int8_t num, uint8_t *p2char, uint8_t pos); //Convertir numero signado de 2 digitos en string.
void num2strUnsigned(uint16_t num, uint8_t *p2char, uint8_t pos); //Convertir numero no signado de 3 digitos en string.
int8_t str2numSigned(uint8_t *p2char, uint8_t pos); //Convertir string en numero signado de 2 digitos.
uint16_t str2numUnsigned(uint8_t *p2char, uint8_t pos); //Convertir string en numero no signado de 3 digitos.

//-------------------------------------------------------------------------------------------------------------------
//Inicializacion de variables
int8_t joystickVal; //Valor del joystick que leo y envío.
uint8_t modoRemoto='N'; //Modo remoto key (Y/N) que recibo por UART.
uint16_t phi1=230; //Angulo de encendido de bujia a recibir por UART y enviar por CAN.
uint16_t phi2=260; //Angulo de apagado de bujia a recibir por UART y enviar por CAN.
int8_t angulo = -90; //Angulo del servo que calculo en base a las rpmMedidas.
uint8_t rpmMedidas; //Lectura de rpm's que recibo por CAN.

uint8_t can_receive_msg_length = 1; //Recibo mensajes de 1 byte (rpmMedidas)
uint8_t can_send_msg_length = 5; //Envio mensajes de 5 bytes (joystickVal, phi1(x2) y phi2(x2)).
uint8_t can_received[1]; //Buffer del mensaje recibido.
uint8_t can_to_send[5]; //Buffer del mensaje a enviar.
uint8_t uart_received_length=10; //Recibo mensajes de 6 chars: phi1(x3), phi2(x3), modoRemoto(x1) y joystickVal(x3).
uint8_t uart_to_send[8]; //Envio mensajes de 7 chars: joystickVal(x3), rpmMedidas(x3) y un terminador (\n).
                         //Agrego ademas un terminador (\0) para que el driver sepa donde frenar.
uint8_t uart_received[10]; //Recibo mensajes de 10 chars: phi1(x3), phi2(x3), modoRemoto(x1), joystickVal(x3).

uint8_t cycle_flag; //Flag que indica que corresponde comenzar un nuevo ciclo.
uint16_t duracion_ciclo_en_50ms = 2; //Quiero hacer un ciclo cada 100ms.

//-------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------
//MAIN
void main(void)
{
    appInit(); //Inicializar drivers.

    while(TRUE)
    {
        appRun(); //Correr en loop la logica del programa
    }
}

//-------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------
//Definicion de funciones
void appInit(void) //Inicializar drivers.
{
    uart_to_send[6]='\n'; //Guardo los terminadores en el mensaje de UART a enviar:
    uart_to_send[7]='\0'; //'\n' para la PC (Matlab) y '\0' para el driver de UART.

    systemInit(MHZ_1, MS_50, RTI_TIMER0); //Stop watchdog, set clock en 1MHz, set rti en 1 interrupt cada 50ms. Rti's con Timer0_A.
    commsInit(uart_received_length); //Inicializo comunicacion UART y CAN.
    joystickInit(ADC_MSK,NULL); //Inicializo joystick en P1.3.
    servoInit(P2_1_SERVO, 1, NULL); //Inicializo servo: controlado por pwm con Timer1_A, conectado a pin P2.1. Sin suscripcion a rti's.

    rtiSubmitCallback(setFlag, duracion_ciclo_en_50ms); //Suscribo la funcion setFlag() para rti's cada 100ms.
    gpioMode(RED_LED_MSK,OUTPUT); //Inicializo el led rojo nativo de la placa,
    gpioWrite(RED_LED_MSK,LOW);   //lo dejo apagado.

    __delay_cycles(DELAY_1s); //Tiempo de espera para que ambas placas se inicialicen.
    enable_interrupts(); //Habilito interrupciones para varios drivers.
}

//-------------------------------------------------------------------------------------------------------------------
void appRun(void) //Correr en loop la logica del programa.
{
    if(cycle_flag)
    {
        cycle_flag=0; //Bajo flag.
        if(modoRemoto!='Y') //Si no estoy en modo remoto
        {
            joystickVal=readJoystick()* JOYSTICK2ACEL; //Leo joystick y escalo la aceleracion a [-10; +10].
        }

        //Armo mensaje de UART: joystickVal (0-2), rpmMedidas (3-5).
        num2strSigned(joystickVal, uart_to_send, 0);
        num2strUnsigned(rpmMedidas, uart_to_send, 3);
        UARTSend(uart_to_send); //Envio el mensaje de UART (6 bytes de info + '\n').

        //Armo mensaje a enviar por CAN: joystickVal (0), phi1 (1-2), phi2 (3-4).
        CANBuildMsg8(can_to_send, joystickVal, 0);
        CANBuildMsg16(can_to_send, phi1, 1);
        CANBuildMsg16(can_to_send, phi2, 3);
        CANSend(can_send_msg_length, can_to_send); //Envio mensaje por CAN (envio 5 byte).

        while(!CANStatus()) //Espero hasta recibir respuesta de CAN.
        {
            //wait
        }
        CANReceive(can_receive_msg_length, can_received); //Recibo mensaje por CAN (recibo un byte).
        rpmMedidas = CANUnbuildMsg8(can_received, 0); //Extraigo rpmMedidas del CAN frame recibido.

        rpmMedidas = suavizarRpm(rpmMedidas); //Promedio los ultimos 4 valores de rpmMedidas para evitar oscilaciones del servo.

        angulo = getAngulo(rpmMedidas); //Mapeo de las rpmMedidas, al angulo que le corresponde al servo (velocimetro).
        servoWrite(angulo); //Actualizo angulo del servo.

        while(!UARTStatus()) //Espero hasta recibir respuesta de UART.
        {
            //wait
        }
        UARTReceive(uart_received); //Recibo mensaje de UART: phi1 (0-2), phi2 (3-5), modoRemoto (6), joystickVal (7-9).

        phi1 = str2numUnsigned(uart_received, 0); //Extraigo phi1 del mensaje de UART.
        phi2 = str2numUnsigned(uart_received, 3); //Extraigo phi2 del mensaje de UART.
        modoRemoto = uart_received[6]; //Extraigo modoRemoto (Y/N) del mensaje de UART.

        if(modoRemoto=='Y') //Si estoy en modo remoto
        {
            joystickVal = str2numSigned(uart_received, 7); //Extraigo joystickVal del mensaje de UART.
        }

        gpioToggle(RED_LED_MSK); //Toggle led rojo para mostrar que se cumplio un ciclo.
    }
}

//-------------------------------------------------------------------------------------------------------------------
void setFlag(void)
{
    cycle_flag=1;
}

//-------------------------------------------------------------------------------------------------------------------
uint8_t suavizarRpm(uint8_t rpm)
{
    static uint8_t yaInit=0;
    static uint8_t rpmVec[4]; //Ultimas 4 lecturas de rpmMedidas recibidas, para ser promediadas.
    static uint8_t pos=0; //Posicion en rpmVec.
    uint16_t rpmAux = 0; //Variable auxiliar para calcular el promedio sin overflows.
    uint8_t i = 0;

    if(!yaInit) //Inicializo rpmVec en {0, 0, 0, 0}.
    {
        for(i=0;i<4;i++)
        {
            rpmVec[i]=0;
        }
        yaInit=1;
    }

    rpmVec[pos]=rpm; //Guardo el ultimo valor de rpm (sobrescribo el mas viejo).
    pos++;
    if(pos==4)
    {
        pos=0;
    }

    for(i=0;i<4;i++) //Calculo el promedio.
    {
        rpmAux += (uint16_t)rpmVec[i];
    }
    rpmAux /= 4;

    return (uint8_t)rpmAux;
}

//-------------------------------------------------------------------------------------------------------------------
int8_t getAngulo(uint8_t rpm) //Mapeo de las rpmMedidas, al angulo que le corresponde al servo (velocimetro).
{
    int8_t ang;
    ang = (int8_t)( (int16_t)(  ( (uint16_t)rpm*180 )  /VEL_MAX)   -90); //Si las rpm van de 0 a 216: 0rpm=-90°, 216rpm=+90°
    if(ang<-90) //Nunca deberia entrar aca, pero por las dudas: evito que se salga del rango -90 a +90.
    {
        ang=-90;
    }
    else if(ang>90)
    {
        ang=90;
    }
    return ang;
}

//-------------------------------------------------------------------------------------------------------------------
void num2strSigned(int8_t num, uint8_t *p2char, uint8_t pos) //Convertir numero signado de 2 digitos en string.
{
    uint8_t i=0;
    uint8_t div=10;
    p2char += pos;

    if(num<0)
    {
        *(p2char+i)='-';
        num *= -1;
    }
    else
    {
        *(p2char+i)='+';
    }
    i++;
    *(p2char+i)=(num/div)+'0';
    i++;
    num -= (num/div)*div;
    *(p2char+i)=num+'0';
}

//-------------------------------------------------------------------------------------------------------------------
void num2strUnsigned(uint16_t num, uint8_t *p2char, uint8_t pos) //Convertir numero no signado de 3 digitos en string.
{
    uint8_t i;
    uint8_t div=100;
    p2char+=pos;

    for(i=0;i<3;i++)
    {
        *(p2char+i)=(num/div)+'0';
        num -= (num/div)*div;
        div /= 10;
    }
}

//-------------------------------------------------------------------------------------------------------------------
int8_t str2numSigned(uint8_t *p2char, uint8_t pos) //Convertir string en numero signado de 2 digitos.
{
    uint8_t i=2;
    int8_t num=0;
    p2char+=pos;

    num += *(p2char+i)-'0';
    i--;
    num += (*(p2char+i)-'0')*10;
    i--;
    if(*(p2char+i)=='-')
    {
        num *= -1;
    }
    return num;
}

//-------------------------------------------------------------------------------------------------------------------
uint16_t str2numUnsigned(uint8_t *p2char, uint8_t pos) //Convertir string en numero no signado de 3 digitos.
{
    uint16_t num=0;
    uint8_t i;
    uint8_t mult=100;
    p2char+=pos;

    for(i=0;i<3;i++)
    {
        num += (*(p2char+i)-'0')*mult;
        mult /= 10;
    }
    return num;
}

//-------------------------------------------------------------------------------------------------------------------
