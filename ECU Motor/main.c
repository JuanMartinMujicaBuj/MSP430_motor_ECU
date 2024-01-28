//CODIGO PARA LA ECU MOTOR
//Maneja un motor dc, un optoacoplador, un led-bujia y comunicacion CAN.

//Inicializo drivers, seteo valores iniciales de variables.

//Recibo lectura del joystick y angulos de encendido y apagado de la bujia por CAN, de la ECU Tablero.
//Leo la velocidad que mide el optoacolpador.
//La envio por CAN, a la ECU Tablero.
//Sumo la aceleracion que marca el joystick a la velocidad que quiero imponerle al motor.
//Se la actualizo al driver del motor dc, para hacerle pwm.
//Actualizo angulos de encendido, apagado y valor de rpm's medido con el opto en el driver de control de la bujia.
//Hago t0do esto en ciclos cada poco mas de 100ms, la velocidad la determinan los mensajes de CAN que recibo.

//En paralelo, hago pwm al motor dc con un timer, mido rpms con el opto con otro timer, y actualizo angulo del motor
//para decidir cuando encender y apagar la bujia con interrupciones de otro timer (wdt).

//-------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------
//Drivers
#include "common.h"
#include "board.h"
#include "system.h"
#include "gpio.h"
#include "comms.h"
#include "motor.h"
#include "bujiaControl.h"
#include "optoacoplador.h"

//-------------------------------------------------------------------------------------------------------------------
//Defines
#define TIMER0 0
#define TIMER1 1

//-------------------------------------------------------------------------------------------------------------------
//Prototipos de funciones
void appInit(void); //Inicializar drivers.
void appRun(void);  //Correr logica del programa dentro de un while(TRUE).

void sincronizarBujia(void); //Funcion para llamar por puntero cuando detecto un rising edge. Sincroniza el angulo
                             //del motor (calculado en el driver bujiaControl) con las ranuras en 0°, 90°, 180°, 270°.
//Funcion para sumar joystickVal a las rpmCalculadas con las que hacer pwm al motor dc
uint8_t getRpmCalculadas(uint8_t rpm, int8_t deltaRpm, uint8_t velMin, uint8_t velMax);

//-------------------------------------------------------------------------------------------------------------------
//Inicializacion de variables
int8_t joystickVal; //Valor del joystick que recibo.
uint16_t phi1; //Valor de phi1 que recibo.
uint16_t phi2; //Valor de phi2 que recibo.
uint8_t rpmCalculadas; //Lectura de rpm's que calculo y con las que hago pwm al motor dc.
uint8_t rpmMedidas; //Lectura de rpm's que mido con el optoacoplador.

uint8_t can_receive_msg_length = 5; //Recibo mensajes de 5 bytes (joystickVal, phi1(x2) y phi2(x2)).
uint8_t can_send_msg_length = 1; //Envio mensajes de 1 byte (rpmMedidas).
uint8_t can_received[5]; //Buffer del mensaje recibido.
uint8_t can_to_send[1]; //Buffer del mensaje a enviar.

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
    systemInit(MHZ_1, MS_05, RTI_WDT); //Stop watchdog, set clock en 1MHz, rti's cada 0.5ms con watchdog timer.
    commsInit(0); //Inicializo comunicacion CAN. Como no quiero comunicacion UART, le paso un 0.
    motorInit(P2_1_DC, TIMER1, NULL); //Inicializo motor dc: le hago pwm con Timer1_A y pin P2.1. No lo suscribo a interrupts periodicas.
    bujiaControlInit(RED_LED_MSK, rpmCalculadas, phi1, phi2); //Inicializo driver que enciende y apaga la bujia en los angulos requeridos.
    optoInit(P1_2_OP, TIMER0, sincronizarBujia, NULL); //Inicializo optoacoplador: hago input capture con Timer0_A, le paso funcion de callback
                                                       //en las capturas, pero no lo suscribo a interrupts periodicas.

    __delay_cycles(DELAY_1s); //Tiempo de espera para que ambas placas se inicialicen.
    enable_interrupts(); //Habilito interrupciones para varios drivers.
}

//-------------------------------------------------------------------------------------------------------------------
void appRun(void) //Correr en loop la logica del programa.
{
    while(!CANStatus()) //Espero que me llegue algun mensaje por CAN.
    {
        //wait
    }
    CANReceive(can_receive_msg_length, can_received); //Recibo mensaje con el valor de joystickVal, phi1, phi2 por CAN (recibo 5 bytes).

    rpmMedidas = readOpto(); //Tomo la ultima lectura de velocidad del optoacoplador.

    CANBuildMsg8(can_to_send, rpmMedidas, 0); //Armo mensaje a enviar
    CANSend(can_send_msg_length, can_to_send); //Envio rpmMedidas por CAN (envio 1 byte)

    //Extraigo joystickVal (byte 0), phi1 (bytes 1-2) y phi2 (bytes 3-4) del CAN frame recibido
    joystickVal = CANUnbuildMsg8(can_received, 0);
    phi1 = CANUnbuildMsg16(can_received, 1);
    phi2 = CANUnbuildMsg16(can_received, 3);

    //Sumo joystickVal a las rpmCalculadas. Permito de 0 a 216rpm (214.9 en realidad, por el 99.5% de duty cycle)
    rpmCalculadas = getRpmCalculadas(rpmCalculadas, joystickVal, 0, DC_VEL_MAX);

    writeMotor((uint16_t)(rpmCalculadas)); //Actualizo velocidad del motor

    bujiaControlWrite(rpmMedidas,phi1,phi2); //Actualizo valores de rpm, phi1, phi2 para el encendido y apagado de la bujia.

    //En simultaneo a este while, funcionan el optoacoplador en un timer, el pwm del motor dc en otro, y la actualizacion
    //del angulo de motor y encendido de bujia con rti's del watchdog timer.
}

//-------------------------------------------------------------------------------------------------------------------
void sincronizarBujia(void) //Funcion para llamar por puntero cuando detecto un rising edge. Sincroniza el angulo
{                           //del motor (calculado en el driver bujiaControl) con las ranuras en 0°, 90°, 180°, 270°.
    static uint16_t cuadrante=0;
    bujiaSetAngle(cuadrante*90);
    cuadrante++;
    if(cuadrante==4)
    {
        cuadrante=0;
    }
}

//-------------------------------------------------------------------------------------------------------------------
uint8_t getRpmCalculadas(uint8_t rpm, int8_t deltaRpm, uint8_t velMin, uint8_t velMax)
{
    if(deltaRpm>=0)
    {
        rpm += (uint8_t)deltaRpm;
        if(rpm>velMax)
        {
            rpm = velMax;
        }
    }
    else //deltaRpml<0
    {
        rpm -= (uint8_t)(-deltaRpm);
        if(rpm>245 | rpm<velMin) //negative overflow
        {
            rpm=0;
        }
    }
    return rpm;
}
