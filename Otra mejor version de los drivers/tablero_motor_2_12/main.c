//prueba 7: uso del input capture con Timer0_A y optoacoplador. Deteccion de flanco
//ascendente, descendente, y ambos, para periodos entre 6.66ms (250rpm) y 166.66ms (10rpm)
/******************************************************************************/

/******************************************************************************/

//prueba 6.3: CODIGO PARA LA PLACA TABLERO
//leo joystick periodicamente, lo envio por CAN del Tablero al Motor, espero hasta recibir un valor de rpm's,
//y con ese valor que recibo, lo transformo a un angulo y muevo el servo.
//También envio por uart el valor del joystick y el valor de rpms, para controlar.

//Y recibo por uart el valor de phi1, phi2, que envio al Motor por CAN.
//Ademas, recibo por uart un joystickVal y un modoRemoto. Si modoRemoto='Y',
//uso el joystickVal de la PC, en vez del del joystick.

//NOTA: NO encontre forma facil de enviar el int8_t o uint8_t de joystick, rpm, angulo.
//En cambio, envio ese valor pasado a string ('+07', '-15', etc).

//Con ciclos de 50ms: anda mejor que el aire acondicionado en los estadios de qatar

//Drivers
#include "common.h"
#include "board.h"
#include "system.h"
#include "rti.h"
#include "gpio.h"
#include "comms.h"
#include "joystick_2.h"
#include "servo.h"

//Var globales
uint8_t cycle_flag;
//------------------------------------------------------------
//Funciones
void setFlag(void)
{
    cycle_flag=1;
}
//------------------------------------------------------------
void num2str(int8_t num, uint8_t *p2char)
{ //sprintf muy sencillo y ad hoc. Convierto +-10 en '+-10'
    uint8_t i=0;
    uint8_t div=10;
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
//------------------------------------------------------------
void num2str_phi(uint16_t num, uint8_t *p2char)
{ //sprintf muy sencillo y ad hoc. Convierto 0 a 200 en '000' a '200'.
    uint8_t i;
    uint8_t div=100;
    for(i=0;i<3;i++)
    {
        *(p2char+i)=(num/div)+'0';
        num -= (num/div)*div;
        div /= 10;
    }
}
//------------------------------------------------------------
int8_t str2num(uint8_t *p2char)
{ //conversion muy sencilla y ad hoc. Convierto '+-90' en +-90
    uint8_t i=2;
    int8_t num=0;
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
//------------------------------------------------------------
uint16_t str2num_phi(uint8_t *p2char)
{ //conversion muy sencilla y ad hoc. Convierto de '0' a '360' en 0 a 360.
    uint16_t num=0;
    uint8_t i;
    uint8_t mult=100;

    for(i=0;i<3;i++)
    {
        num += (*(p2char+i)-'0')*mult;
        mult /= 10;
    }
    return num;
}
//------------------------------------------------------------

//MAIN
void main(void)
{
    uint16_t ciclos_cada_50ms = 2; //es el inverso de ese valor, quiero hacer un ciclo cada 100ms
    int8_t joystickVal; //valor del joystick que leo y envío.
    uint8_t modoRemoto='N'; //modo remoto key (Y/N) que recibo por uart.
    uint16_t phi1=160; //angulo de encendido de bujia a recibir por uart y enviar por CAN
    uint16_t phi2=180; //angulo de apagado de bujia a recibir por uart y enviar por CAN
    int8_t angulo = 0; //ángulo del servo que calculo
    uint8_t rpmMedidas = 0; //lectura de rpm's que recibo.
    uint8_t can_receive_msg_length = 1; //voy a recibir 1 byte con la lectura del joystick
    uint8_t can_send_msg_length = 5; //voy a enviar 5 bytes con el valor de rpm's, phi1 y phi2.
    uint8_t can_to_send[5];
    uint8_t uart_received_length=10; //voy a recibir un mensaje de 6 chars: 3 de phi1, 3 de phi2, 1 de modoRemoto, 3 de joystickVal
    uint8_t uart_to_send[8]; //mensaje que envio por uart para chequear que tengo valores coherentes.
    uint8_t uart_received[10];
    uint8_t i,j;
    uint8_t rpmVec[4];
    uint16_t rpmAux;
    uint8_t counter = 0;
    uint8_t rpmAVGSamples = 4;

    for(i=0;i<6;i++) //inicializo el msj de uart a enviar con ceros (y los terminadores: '\n' para matlab y '\0' para el driver de uart)
    {
        uart_to_send[i]='0';
    }
    uart_to_send[6]='\n';
    uart_to_send[7]='\0';

    systemInit(MHZ_1, MS_50, RTI_TIMER0); //stop watchdog, set clock in 1MHz, set rti in 1 interrupt/50ms. Rti's with Timer0_A.
    commsInit(uart_received_length); //inicializo comunicacion uart y CAN.
    joystickInit(ADC_MSK); //joystick en P1.3
    servoInit(1, 0, P2_1_SERVO); //pwm con Timer1_A, servo conectado a pin P2.1

    rtiSubmitCallback(setFlag, ciclos_cada_50ms);
    gpioMode(RED_LED_MSK,OUTPUT);
    gpioWrite(RED_LED_MSK,LOW);

    __delay_cycles(DELAY_1s); //para darle tiempo a ambas placas de inicializarse
    enable_interrupts();
    while(TRUE)
    {
        if(cycle_flag)
        {
            cycle_flag=0; //bajo flag
            if(modoRemoto!='Y') //si no estoy en modo remoto
            {
                joystickVal=joystickRead(); //leo joystick (-10 a +10)
            }

            num2str(joystickVal, uart_to_send); //armo mensaje de uart: joystickVal (0-2), rpmMedidas (3-5)
            num2str_phi(rpmMedidas, &(uart_to_send[3]));
            UARTSend(uart_to_send); //envio el mensaje de uart (12 bytes de info + '\n' + '\0')

            can_to_send[0]=joystickVal; //armo mensaje de CAN: joystickVal (0), phi1 (1-2), phi2 (3-4)
            can_to_send[1]=(uint8_t)(phi1/256);
            can_to_send[2]=(uint8_t)(phi1-(phi1/256)*256);
            can_to_send[3]=(uint8_t)(phi2/256);
            can_to_send[4]=(uint8_t)(phi2-(phi2/256)*256);
            CANSend(can_send_msg_length,can_to_send); //envio el mensaje de CAN

            while(!CANStatus()) //espero hasta recibir respuesta de CAN
            {
                //wait
            }
            CANReceive(can_receive_msg_length,&rpmMedidas); //recibo lectura de rpm's por CAN (recibo un byte)
            //rpmMedidas=215;

            rpmVec[counter]=rpmMedidas;
            counter++;
            if(counter==rpmAVGSamples)
            {
                counter=0;
            }
            rpmAux=0;
            for (j=0;j<rpmAVGSamples;j++)
            {
               rpmAux+=(uint16_t)rpmVec[j];
            }
            rpmAux /= rpmAVGSamples;
            rpmMedidas = (uint8_t)rpmAux;



            angulo = (int8_t)( (int16_t)(  ( (uint16_t)rpmMedidas*180 )  /200)   -90); //si las rpm van de 0 a 200: 0rpm=-90°, 200rpm=+90°
            if(angulo<-90) //nunca deberia entrar aca, pero por las dudas: evito que se salga del rango -90 a +90
            {
                angulo=-90;
            }
            else if(angulo>90)
            {
                angulo=90;
            }


            servoWrite(angulo); //actualizo angulo del servo

            while(!UARTStatus()) //espero hasta recibir respuesta de uart
            {
                //wait
            }
            UARTReceive(uart_received); //recibir phi1, phi2, modoRemoto, joystickVal por uart y guardarlo adonde apunta ese p2char
            phi1 = str2num_phi(uart_received); //convertir los 3 primeros chars en un numero (phi1)
            phi2 = str2num_phi(&(uart_received[3])); //convertir los 3 siguientes chars en un numero (phi2)
            modoRemoto = uart_received[6]; //extraer el siguiente char, que sera un 'Y' o un 'N'
            if(modoRemoto=='Y') //si estoy en modo remoto
            {
                joystickVal = str2num(&(uart_received[7])); //convertir los 3 ultimos chars en un numero (joystickVal)
            }

            gpioToggle(RED_LED_MSK); //toggle led para ver que se cumplio un ciclo :)
        }
    }
}

/******************************************************************************/
/*
//prueba 6.3: CODIGO PARA LA PLACA MOTOR
//recibo lectura del joystick por CAN del Tablero al Motor, se lo sumo a las rpm actuales y envio por CAN ese valor de rpm's.
//Con el valor que tengo de rpm's, actuo sobre el pwm del pin 1.6 con dc_motor.h.
//Además, recibo valores de phi1, phi2 por CAN. Segun mi phi actual, que actualizo cada 0.5ms, veo si prender o
//apagar el led de bujia con bujia.h

//Con ciclos de 50ms en Tablero y actualizaciones de phi cada 0.5ms en Motor: anda bien! esta al limite con los 0.5ms pero va!

//Drivers
#include "common.h"
#include "board.h"
#include "system.h"
#include "gpio.h"
#include "comms.h"
#include "dc_motor.h"
#include "bujia.h"
#include "optoacoplador.h"

//Var globales
uint16_t cuadrante=0;
//------------------------------------------------------------
//Funciones
void sincronizarBujia(void) //Funcion llamada por puntero cuando detecto un rising edge. Sincroniza el angulo del motor con las ranurasen 0°, 90°, 180°, 270°.la bujia
{
    bujiaSetAngle(cuadrante*90);
    cuadrante++;
    if(cuadrante==4)
    {
        cuadrante=0;
    }
}
//------------------------------------------------------------

//MAIN
void main(void)
{
    uint8_t can_receive_msg_length = 5; //recibo mensajes de 5 bytes (joystickVal, phi1(x2), phi2(x2))
    uint8_t can_send_msg_length = 1; //envio mensajes de 1 byte (rpm)
    uint8_t can_received[5];

    int8_t joystickVal=0; //valor del joystick que recibo.
    uint16_t phi1=150; //valor de phi1 que recibo
    uint16_t phi2=180; //valor de phi2 que recibo
    uint8_t rpmCalculadas = 150; //lectura de rpm's que calculo.
    uint8_t rpmMedidas = 150; //lectura de rpm's que mido (cuando tenga optoacoplador) y envío.
    uint16_t velMotor_x10 = 1500; //valor en [rpm*10] que quiero del motor dc, al que le hago pwm.

    systemInit(MHZ_1, MS_05, RTI_WDT); //stop watchdog, set clock in 1MHz, rti's every 0.5ms with watchdog timer.
    commsInit(0); //uart_msg_length: no me interesa, le pongo 0.
    dcMotorInit(1, velMotor_x10, P2_1_DC); //inicializar dc_motor: con timer1_A y pin P2.1
    bujiaInit(RED_LED_MSK, rpmCalculadas, phi1, phi2); //inicializar driver que enciende y apaga la bujia
    //gpioMode(RED_LED_MSK,OUTPUT);
    optoInit(P1_2_OP,0,sincronizarBujia,NULL);

    __delay_cycles(DELAY_1s); //para darle tiempo a ambas placas de inicializarse
    enable_interrupts();
    while(TRUE)
    {
        while(!CANStatus()) //espero que me llegue algun mensaje con joystickVal
        {
            //wait
        }
        CANReceive(can_receive_msg_length,can_received); //recibo valor de joystickVal, phi1, phi2 por CAN (recibo 5 bytes)

        rpmMedidas = readOpto(); //aca en realidad iria una func del optoacoplador
        //rpmMedidas = rpmCalculadas;

        CANSend(can_send_msg_length,&rpmMedidas); //envio valor medido de rpm's por CAN (envio un byte, que será interpretado como uint8_t)

        joystickVal=can_received[0]; //el primer byte es joystickVal
        phi1=(uint16_t)can_received[1]*256+(uint16_t)can_received[2];        //el segundo y tercer byte son phi1
        phi2=(uint16_t)can_received[3]*256+(uint16_t)can_received[4];        //el cuarto y quinto byte son phi2

        if(joystickVal>=0) //sumo joystickVal a las rpm's. Permito de 50 a 216rpm (214.9 en realidad, por el 99.5% de duty cycle)
        {
            rpmCalculadas += (uint8_t)joystickVal;
            if(rpmCalculadas>216)
            {
                rpmCalculadas=216;
            }
        }
        else //joystickVal<0
        {
            rpmCalculadas -= (uint8_t)(-joystickVal);
            if(rpmCalculadas>245 | rpmCalculadas<10) //negative overflow
            {
                rpmCalculadas=10;
            }
        }

        velMotor_x10 = (uint16_t)(rpmCalculadas)*10; //calculo y actualizo
        dcMotorWrite(velMotor_x10);                  //velocidad del motor

        bujiaWrite(rpmMedidas,phi1,phi2); //actualizo valores de rpm,phi1,phi2 para el encendido y apagado de la bujia
        //cuando tenga el optoacoplador, al input capture le puedo pasar la funcion bujiaZero()
        //gpioToggle(RED_LED_MSK);
    }
}
*/
/******************************************************************************/

//prueba 7: uso del input capture con Timer0_A y optoacoplador. Deteccion de flanco
//ascendente, descendente, y ambos, para periodos entre 6.66ms (250rpm) y 166.66ms (10rpm)
/******************************************************************************/

/******************************************************************************/

//prueba 6.2: CODIGO PARA LA PLACA TABLERO
//leo joystick periodicamente, lo envio por CAN del Tablero al Motor, espero hasta recibir un valor de rpm's,
//y con ese valor que recibo, lo transformo a un angulo y muevo el servo.
//También envio por uart el valor del joystick y el valor de rpms, para controlar.

//Y recibo por uart el valor de phi1, phi2, que envio al Motor por CAN.
//Ademas, recibo por uart un joystickVal y un modoRemoto. Si modoRemoto='Y',
//uso el joystickVal de la PC, en vez del del joystick.

//NOTA: NO encontre forma facil de enviar el int8_t o uint8_t de joystick, rpm, angulo.
//En cambio, envio ese valor pasado a string ('+07', '-15', etc).

//Con ciclos de 50ms: anda mejor que el aire acondicionado en los estadios de qatar
/*
//Drivers
#include "common.h"
#include "board.h"
#include "system.h"
#include "rti.h"
#include "gpio.h"
#include "comms.h"
#include "joystick_2.h"
#include "servo.h"

//Var globales
uint8_t cycle_flag;
//------------------------------------------------------------
//Funciones
void setFlag(void)
{
    cycle_flag=1;
}
//------------------------------------------------------------
void num2str(int8_t num, uint8_t *p2char)
{ //sprintf muy sencillo y ad hoc. Convierto +-10 en '+-10'
    uint8_t i=0;
    uint8_t div=10;
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
//------------------------------------------------------------
void num2str_phi(uint16_t num, uint8_t *p2char)
{ //sprintf muy sencillo y ad hoc. Convierto 0 a 200 en '000' a '200'.
    uint8_t i;
    uint8_t div=100;
    for(i=0;i<3;i++)
    {
        *(p2char+i)=(num/div)+'0';
        num -= (num/div)*div;
        div /= 10;
    }
}
//------------------------------------------------------------
int8_t str2num(uint8_t *p2char)
{ //conversion muy sencilla y ad hoc. Convierto '+-90' en +-90
    uint8_t i=2;
    int8_t num=0;
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
//------------------------------------------------------------
uint16_t str2num_phi(uint8_t *p2char)
{ //conversion muy sencilla y ad hoc. Convierto de '0' a '360' en 0 a 360.
    uint16_t num=0;
    uint8_t i;
    uint8_t mult=100;

    for(i=0;i<3;i++)
    {
        num += (*(p2char+i)-'0')*mult;
        mult /= 10;
    }
    return num;
}
//------------------------------------------------------------

//MAIN
void main(void)
{
    uint16_t ciclos_cada_50ms = 1; //quiero hacer un ciclo cada 50ms
    int8_t joystickVal; //valor del joystick que leo y envío.
    uint8_t modoRemoto='N'; //modo remoto key (Y/N) que recibo por uart.
    uint16_t phi1=160; //angulo de encendido de bujia a recibir por uart y enviar por CAN
    uint16_t phi2=180; //angulo de apagado de bujia a recibir por uart y enviar por CAN
    int8_t angulo = 0; //ángulo del servo que calculo
    uint8_t rpmMedidas = 0; //lectura de rpm's que recibo.
    uint8_t can_receive_msg_length = 1; //voy a recibir 1 byte con la lectura del joystick
    uint8_t can_send_msg_length = 5; //voy a enviar 5 bytes con el valor de rpm's, phi1 y phi2.
    uint8_t can_to_send[5];
    uint8_t uart_received_length=10; //voy a recibir un mensaje de 6 chars: 3 de phi1, 3 de phi2, 1 de modoRemoto, 3 de joystickVal
    uint8_t uart_to_send[14]; //mensaje que envio por uart para chequear que tengo valores coherentes.
    uint8_t uart_received[10];
    uint8_t i;

    for(i=0;i<12;i++) //inicializo el msj de uart a enviar con ceros (y los terminadores: '\n' para matlab y '\0' para el driver de uart)
    {
        uart_to_send[i]='0';
    }
    uart_to_send[12]='\n';
    uart_to_send[13]='\0';

    systemInit(MHZ_1, MS_50, RTI_TIMER0); //stop watchdog, set clock in 1MHz, set rti in 1 interrupt/50ms. Rti's with Timer0_A.
    commsInit(uart_received_length); //inicializo comunicacion uart y CAN.
    joystickInit(ADC_MSK); //joystick en P1.3
    servoInit(1, 0, P2_1_SERVO); //pwm con Timer1_A, servo conectado a pin P2.1

    rtiSubmitCallback(setFlag, ciclos_cada_50ms);
    gpioMode(RED_LED_MSK,OUTPUT);
    gpioWrite(RED_LED_MSK,LOW);

    __delay_cycles(DELAY_1s); //para darle tiempo a ambas placas de inicializarse
    enable_interrupts();
    while(TRUE)
    {
        if(cycle_flag)
        {
            cycle_flag=0; //bajo flag
            if(modoRemoto!='Y') //si no estoy en modo remoto
            {
                joystickVal=joystickRead(); //leo joystick (-10 a +10)
            }

            num2str(joystickVal, uart_to_send); //armo mensaje de uart: joystickVal (0-2), rpmMedidas (3-5), phi1 (6-8), phi2 (9-11)
            num2str_phi(rpmMedidas, &(uart_to_send[3]));
            num2str_phi(phi1, &(uart_to_send[6]));
            num2str_phi(phi2, &(uart_to_send[9]));
            UARTSend(uart_to_send); //envio el mensaje de uart (12 bytes de info + '\n' + '\0')

            can_to_send[0]=joystickVal; //armo mensaje de CAN: joystickVal (0), phi1 (1-2), phi2 (3-4)
            can_to_send[1]=(uint8_t)(phi1/256);
            can_to_send[2]=(uint8_t)(phi1-(phi1/256)*256);
            can_to_send[3]=(uint8_t)(phi2/256);
            can_to_send[4]=(uint8_t)(phi2-(phi2/256)*256);
            CANSend(can_send_msg_length,can_to_send); //envio el mensaje de CAN

            while(!CANStatus()) //espero hasta recibir respuesta de CAN
            {
                //wait
            }
            CANReceive(can_receive_msg_length,&rpmMedidas); //recibo lectura de rpm's por CAN (recibo un byte)
            //rpmMedidas=215;

            angulo = (int8_t)( (int16_t)(  ( (uint16_t)rpmMedidas*180 )  /200)   -90); //si las rpm van de 0 a 200: 0rpm=-90°, 200rpm=+90°
            if(angulo<-90) //nunca deberia entrar aca, pero por las dudas: evito que se salga del rango -90 a +90
            {
                angulo=-90;
            }
            else if(angulo>90)
            {
                angulo=90;
            }
            servoWrite(angulo); //actualizo angulo del servo

            while(!UARTStatus()) //espero hasta recibir respuesta de uart
            {
                //wait
            }
            UARTReceive(uart_received); //recibir phi1, phi2, modoRemoto, joystickVal por uart y guardarlo adonde apunta ese p2char
            phi1 = str2num_phi(uart_received); //convertir los 3 primeros chars en un numero (phi1)
            phi2 = str2num_phi(&(uart_received[3])); //convertir los 3 siguientes chars en un numero (phi2)
            modoRemoto = uart_received[6]; //extraer el siguiente char, que sera un 'Y' o un 'N'
            if(modoRemoto=='Y') //si estoy en modo remoto
            {
                joystickVal = str2num(&(uart_received[7])); //convertir los 3 ultimos chars en un numero (joystickVal)
            }

            gpioToggle(RED_LED_MSK); //toggle led para ver que se cumplio un ciclo :)
        }
    }
}
*/
/******************************************************************************/
/*
//prueba 6: CODIGO PARA LA PLACA MOTOR
//recibo lectura del joystick por CAN del Tablero al Motor, se lo sumo a las rpm actuales y envio por CAN ese valor de rpm's.
//Con el valor que tengo de rpm's, actuo sobre el pwm del pin 1.6 con dc_motor.h.
//Además, recibo valores de phi1, phi2 por CAN. Segun mi phi actual, que actualizo cada 0.5ms, veo si prender o
//apagar el led de bujia con bujia.h

//Con ciclos de 50ms en Tablero y actualizaciones de phi cada 0.5ms en Motor: anda bien! esta al limite con los 0.5ms pero va!

//Drivers
#include "common.h"
#include "board.h"
#include "system.h"
#include "gpio.h"
#include "comms.h"
#include "dc_motor.h"
#include "bujia.h"

//MAIN
void main(void)
{
    uint8_t can_receive_msg_length = 5; //recibo mensajes de 5 bytes (joystickVal, phi1(x2), phi2(x2))
    uint8_t can_send_msg_length = 1; //envio mensajes de 1 byte (rpm)
    uint8_t can_received[5];

    int8_t joystickVal=0; //valor del joystick que recibo.
    uint16_t phi1=150; //valor de phi1 que recibo
    uint16_t phi2=180; //valor de phi2 que recibo
    uint8_t rpmCalculadas = 150; //lectura de rpm's que calculo.
    uint8_t rpmMedidas = 150; //lectura de rpm's que mido (cuando tenga optoacoplador) y envío.
    uint16_t velMotor_x10 = 1500; //valor en [rpm*10] que quiero del motor dc, al que le hago pwm.

    systemInit(MHZ_1, MS_05, RTI_WDT); //stop watchdog, set clock in 1MHz, rti's every 0.5ms with watchdog timer.
    commsInit(0); //uart_msg_length: no me interesa, le pongo 0.
    dcMotorInit(1, velMotor_x10, P2_1_DC); //inicializar dc_motor: con timer1_A y pin P2.1
    bujiaInit(RED_LED_MSK, rpmCalculadas, phi1, phi2); //inicializar driver que enciende y apaga la bujia
    //gpioMode(RED_LED_MSK,OUTPUT);

    __delay_cycles(DELAY_1s); //para darle tiempo a ambas placas de inicializarse
    enable_interrupts();
    while(TRUE)
    {
        while(!CANStatus()) //espero que me llegue algun mensaje con joystickVal
        {
            //wait
        }
        CANReceive(can_receive_msg_length,can_received); //recibo valor de joystickVal, phi1, phi2 por CAN (recibo 5 bytes)

        rpmMedidas = rpmCalculadas; //aca en realidad iria una func del optoacoplador

        CANSend(can_send_msg_length,&rpmMedidas); //envio valor medido de rpm's por CAN (envio un byte, que será interpretado como uint8_t)

        joystickVal=can_received[0]; //el primer byte es joystickVal
        phi1=(uint16_t)can_received[1]*256+(uint16_t)can_received[2];        //el segundo y tercer byte son phi1
        phi2=(uint16_t)can_received[3]*256+(uint16_t)can_received[4];        //el cuarto y quinto byte son phi2

        if(joystickVal>=0) //sumo joystickVal a las rpm's. Permito de 50 a 216rpm (214.9 en realidad, por el 99.5% de duty cycle)
        {
            rpmCalculadas += (uint8_t)joystickVal;
            if(rpmCalculadas>216)
            {
                rpmCalculadas=216;
            }
        }
        else //joystickVal<0
        {
            rpmCalculadas -= (uint8_t)(-joystickVal);
            if(rpmCalculadas>245 | rpmCalculadas<50) //negative overflow
            {
                rpmCalculadas=50;
            }
        }

        velMotor_x10 = (uint16_t)(rpmCalculadas)*10; //calculo y actualizo
        dcMotorWrite(velMotor_x10);                  //velocidad del motor

        bujiaWrite(rpmMedidas,phi1,phi2); //actualizo valores de rpm,phi1,phi2 para el encendido y apagado de la bujia
        //cuando tenga el optoacoplador, al input capture le puedo pasar la funcion bujiaZero()
        //gpioToggle(RED_LED_MSK);
    }
}
*/
/******************************************************************************/
/*
//prueba 6: CODIGO PARA LA PLACA TABLERO
//leo joystick periodicamente, lo envio por CAN del Tablero al Motor, espero hasta recibir un valor de rpm's,
//y con ese valor que recibo, lo transformo a un angulo y muevo el servo.
//También envio por uart el valor del joystick y el valor de rpms, para controlar.

//Y recibo por uart el valor de phi1, phi2, que envio al Motor por CAN.

//NOTA: NO encontre forma facil de enviar el int8_t o uint8_t de joystick, rpm, angulo.
//En cambio, envio ese valor pasado a string ('+07', '-15', etc).

//Con ciclos de 50ms: anda joyaaa

//Drivers
#include "common.h"
#include "board.h"
#include "system.h"
#include "rti.h"
#include "gpio.h"
#include "comms.h"
#include "joystick_2.h"
#include "servo.h"

//Var globales
uint8_t cycle_flag;
//------------------------------------------------------------
//Funciones
void setFlag(void)
{
    cycle_flag=1;
}
//------------------------------------------------------------
void num2str(int8_t num, uint8_t *p2char)
{ //sprintf muy sencillo y ad hoc. Convierto +-10 en '+-10'
    uint8_t i=0;
    uint8_t div=10;
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
//------------------------------------------------------------
void num2str_phi(uint16_t num, uint8_t *p2char)
{ //sprintf muy sencillo y ad hoc. Convierto 0 a 200 en '000' a '200'.
    uint8_t i;
    uint8_t div=100;
    for(i=0;i<3;i++)
    {
        *(p2char+i)=(num/div)+'0';
        num -= (num/div)*div;
        div /= 10;
    }
}
//------------------------------------------------------------
int8_t str2num(uint8_t *p2char)
{ //conversion muy sencilla y ad hoc. Convierto '+-90' en +-90
    uint8_t i=2;
    int8_t num=0;
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
//------------------------------------------------------------
uint16_t str2num_phi(uint8_t *p2char)
{ //conversion muy sencilla y ad hoc. Convierto de '0' a '360' en 0 a 360.
    uint16_t num=0;
    uint8_t i;
    uint8_t mult=100;

    for(i=0;i<3;i++)
    {
        num += (*(p2char+i)-'0')*mult;
        mult /= 10;
    }
    return num;
}
//------------------------------------------------------------

//MAIN
void main(void)
{
    uint16_t ciclos_cada_50ms = 1; //quiero hacer un ciclo cada 50ms
    int8_t joystickVal; //valor del joystick que leo y envío.
    uint16_t phi1=160; //angulo de encendido de bujia a recibir por uart y enviar por CAN
    uint16_t phi2=180; //angulo de apagado de bujia a recibir por uart y enviar por CAN
    int8_t angulo = 0; //ángulo del servo que calculo
    uint8_t rpmMedidas = 0; //lectura de rpm's que recibo.
    uint8_t can_receive_msg_length = 1; //voy a recibir 1 byte con la lectura del joystick
    uint8_t can_send_msg_length = 5; //voy a enviar 5 bytes con el valor de rpm's, phi1 y phi2.
    uint8_t can_to_send[5];
    uint8_t uart_received_length=6; //voy a recibir un mensaje de 6 chars: tres de phi1, tres de phi2
    uint8_t uart_to_send[14]; //mensaje que envio por uart para chequear que tengo valores coherentes.
    uint8_t uart_received[6];
    uint8_t i;

    for(i=0;i<12;i++) //inicializo el msj de uart a enviar con ceros (y los terminadores: '\n' para matlab y '\0' para el driver de uart)
    {
        uart_to_send[i]='0';
    }
    uart_to_send[12]='\n';
    uart_to_send[13]='\0';

    systemInit(MHZ_1, MS_50, RTI_TIMER0); //stop watchdog, set clock in 1MHz, set rti in 1 interrupt/50ms. Rti's with Timer0_A.
    commsInit(uart_received_length); //inicializo comunicacion uart y CAN.
    joystickInit(ADC_MSK); //joystick en P1.3
    servoInit(1, 0, P2_1_SERVO); //pwm con Timer1_A, servo conectado a pin P2.1

    rtiSubmitCallback(setFlag, ciclos_cada_50ms);
    gpioMode(RED_LED_MSK,OUTPUT);
    gpioWrite(RED_LED_MSK,LOW);

    __delay_cycles(DELAY_1s); //para darle tiempo a ambas placas de inicializarse
    enable_interrupts();
    while(TRUE)
    {
        if(cycle_flag)
        {
            cycle_flag=0; //bajo flag
            joystickVal=joystickRead(); //leo joystick (-10 a +10)

            num2str(joystickVal, uart_to_send); //armo mensaje de uart: joystickVal (0-2), rpmMedidas (3-5), phi1 (6-8), phi2 (9-11)
            num2str_phi(rpmMedidas, &(uart_to_send[3]));
            num2str_phi(phi1, &(uart_to_send[6]));
            num2str_phi(phi2, &(uart_to_send[9]));
            UARTSend(uart_to_send); //envio el mensaje de uart (12 bytes de info + '\n' + '\0')

            can_to_send[0]=joystickVal; //armo mensaje de CAN: joystickVal (0), phi1 (1-2), phi2 (3-4)
            can_to_send[1]=(uint8_t)(phi1/256);
            can_to_send[2]=(uint8_t)(phi1-(phi1/256)*256);
            can_to_send[3]=(uint8_t)(phi2/256);
            can_to_send[4]=(uint8_t)(phi2-(phi2/256)*256);
            CANSend(can_send_msg_length,can_to_send); //envio el mensaje de CAN

            while(!CANStatus()) //espero hasta recibir respuesta de CAN
            {
                //wait
            }
            CANReceive(can_receive_msg_length,&rpmMedidas); //recibo lectura de rpm's por CAN (recibo un byte)
            //rpmMedidas=215;

            angulo = (int8_t)( (int16_t)(  ( (uint16_t)rpmMedidas*180 )  /200)   -90); //si las rpm van de 0 a 200: 0rpm=-90°, 200rpm=+90°
            if(angulo<-90) //nunca deberia entrar aca, pero por las dudas: evito que se salga del rango -90 a +90
            {
                angulo=-90;
            }
            else if(angulo>90)
            {
                angulo=90;
            }
            servoWrite(angulo); //actualizo angulo del servo

            while(!UARTStatus()) //espero hasta recibir respuesta de uart
            {
                //wait
            }
            UARTReceive(uart_received); //recibir phi1, phi2 por uart y guardarlo adonde apunta ese p2char
            phi1 = str2num_phi(uart_received); //convertir los 3 primeros chars en un numero (phi1)
            phi2 = str2num_phi(&(uart_received[3])); //convertir los 3 ultimos chars en un numero (phi2)

            gpioToggle(RED_LED_MSK); //toggle led para ver que se cumplio un ciclo :)
        }
    }
}
*/
/******************************************************************************/
/*
//prueba 5: CODIGO PARA LA PLACA MOTOR!!!
//recibo lectura del joystick por CAN del Tablero al Motor, se lo sumo a las rpm actuales y envio por CAN ese valor de rpm's.
//Con el valor que tengo de rpm's, actuo sobre el pwm del pin 1.6 con dc_motor.h.
//Con ciclos de 50ms: ANDA EXCELENTE

//Drivers
#include "common.h"
#include "board.h"
#include "system.h"
#include "gpio.h"
#include "comms.h"
#include "dc_motor.h"

//Var globales
uint8_t cycle_flag;
//------------------------------------------------------------
//Funciones
void setFlag(void)
{
    cycle_flag=1;
}
//------------------------------------------------------------
void num2str(int8_t num, uint8_t *p2char)
{ //sprintf muy sencillo y ad hoc. Convierto +-10 en '+-10'
    uint8_t i=0;
    uint8_t div=10;
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
//------------------------------------------------------------
void num2str_rpm(uint8_t num, uint8_t *p2char)
{ //sprintf muy sencillo y ad hoc. Convierto 0 a 200 en '000' a '200'.
    uint8_t i;
    uint8_t div=100;
    for(i=0;i<3;i++)
    {
        *(p2char+i)=(num/div)+'0';
        num -= (num/div)*div;
        div /= 10;
    }
}
//------------------------------------------------------------
int8_t str2num(uint8_t *p2char)
{ //conversion muy sencilla y ad hoc. Convierto '+-90' en +-90
    uint8_t i=2;
    int8_t num=0;
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
//------------------------------------------------------------

//MAIN
void main(void)
{
    uint8_t can_msg_length = 1; //recibo y envío mensajes de 1 byte
    int8_t joystickVal; //valor del joystick que recibo.
    uint8_t rpm = 150; //lectura de rpm's que calculo y envío.
    uint16_t velMotor_x10 = 1500; //valor en [rpm*10] que quiero del motor dc, al que le hago pwm.

    systemInit(MHZ_1, MS_50, NO_RTI); //stop watchdog, set clock in 1MHz, no rti's.
    commsInit(0); //uart_msg_length: no me interesa, le pongo 0.
    dcMotorInit(1, velMotor_x10, P2_1_DC); //inicializar dc_motor: con timer1_A y pin P2.1

    gpioMode(RED_LED_MSK,OUTPUT);
    gpioWrite(RED_LED_MSK,LOW);

    enable_interrupts();
    while(TRUE)
    {
        while(!CANStatus()) //espero que me llegue algun mensaje con joystickVal
        {
            //wait
        }
        CANReceive(can_msg_length,&joystickVal); //recibo valor de joystickVal por CAN (recibo un byte, que interpreto como int8_t)
        if(joystickVal>=0) //sumo joystickVal a las rpm's. Permito de 100 a 216rpm (214.9 en realidad, por el 99.5% de duty cycle)
        {
            rpm += (uint8_t)joystickVal;
            if(rpm>216)
            {
                rpm=216;
            }
        }
        else //joystickVal<0
        {
            rpm -= (uint8_t)(-joystickVal);
            if(rpm>245 | rpm<100) //negative overflow
            {
                rpm=100;
            }
        }

        CANSend(can_msg_length,&rpm); //envio valor calculado de rpm's por CAN (envio un byte, que será interpretado como uint8_t)
        velMotor_x10 = (uint16_t)(rpm)*10; //calculo y actualizo
        dcMotorWrite(velMotor_x10);        //velocidad del motor

        gpioToggle(RED_LED_MSK); //toggle led para ver que se cumplio un ciclo :)
    }
}
*/
/******************************************************************************/
/*
//prueba 5: CODIGO PARA LA PLACA TABLERO!!!
//leo joystick periodicamente, lo envio por CAN del Tablero al Motor, espero hasta recibir un valor de rpm's,
//y con ese valor que recibo, lo transformo a un angulo y muevo el servo.
//También envio por uart el valor del joystick y el valor de rpms, para controlar.
//NOTA: NO encontre forma facil de enviar el int8_t o uint8_t de joystick, rpm, angulo.
//En cambio, envio ese valor pasado a string ('+07', '-15', etc).
//Con ciclos de 50ms: ANDA EXCELENTE

//Drivers
#include "common.h"
#include "board.h"
#include "system.h"
#include "rti.h"
#include "gpio.h"
#include "comms.h"
#include "joystick_2.h"
#include "servo.h"

//Var globales
uint8_t cycle_flag;
//------------------------------------------------------------
//Funciones
void setFlag(void)
{
    cycle_flag=1;
}
//------------------------------------------------------------
void num2str(int8_t num, uint8_t *p2char)
{ //sprintf muy sencillo y ad hoc. Convierto +-10 en '+-10'
    uint8_t i=0;
    uint8_t div=10;
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
//------------------------------------------------------------
void num2str_rpm(uint8_t num, uint8_t *p2char)
{ //sprintf muy sencillo y ad hoc. Convierto 0 a 200 en '000' a '200'.
    uint8_t i;
    uint8_t div=100;
    for(i=0;i<3;i++)
    {
        *(p2char+i)=(num/div)+'0';
        num -= (num/div)*div;
        div /= 10;
    }
}
//------------------------------------------------------------
int8_t str2num(uint8_t *p2char)
{ //conversion muy sencilla y ad hoc. Convierto '+-90' en +-90
    uint8_t i=2;
    int8_t num=0;
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
//------------------------------------------------------------

//MAIN
void main(void)
{
    uint16_t ciclos_cada_50ms = 1; //quiero hacer un ciclo cada 50ms
    int8_t joystickVal; //valor del joystick que leo y envío.
    int8_t angulo = 0; //ángulo del servo que calculo
    uint8_t rpm = 0; //lectura de rpm's que recibo.
    uint8_t can_msg_length = 1; //voy a enviar 1 byte con la lectura del joystick, y recibir 1 byte con el valor de rpm's
    uint8_t uart_msg[11]; //mensaje que envio por uart para chequear que tengo valores coherentes.
    uint8_t i;

    for(i=0;i<9;i++) //inicializo el msj de uart con ceros (y los terminadores: '\n' para matlab y '\0' para el driver de uart)
    {
        uart_msg[i]='0';
    }
    uart_msg[9]='\n';
    uart_msg[10]='\0';

    systemInit(MHZ_1, MS_50, RTI_TIMER0); //stop watchdog, set clock in 1MHz, set rti in 1 interrupt/50ms. Rti's with Timer0_A.
    commsInit(0); //uart_msg_length: no me interesa recibir, le pongo 0.
    joystickInit(ADC_MSK); //joystick en P1.3
    servoInit(1, 0, P2_1_SERVO); //pwm con Timer1_A, servo conectado a pin P2.1

    rtiSubmitCallback(setFlag, ciclos_cada_50ms);
    gpioMode(RED_LED_MSK,OUTPUT);
    gpioWrite(RED_LED_MSK,LOW);

    enable_interrupts();
    while(TRUE)
    {
        if(cycle_flag)
        {
            cycle_flag=0; //bajo flag
            joystickVal=joystickRead(); //leo joystick (-10 a +10)
            CANSend(can_msg_length,&joystickVal); //envio joystickVal por CAN (envio un char/uint8_t/int8_t/un byte,
            while(!CANStatus())                   // que debera ser interpretado como un int8_t)
            {
                //wait
            }
            CANReceive(can_msg_length,&rpm); //recibo lectura de rpm's por CAN (recibo un char/uint8_t/un byte, que interpreto como uint8_t)

            angulo = (int8_t)( (int16_t)(  ( (uint16_t)rpm*180 )  /200)   -90); //si las rpm van de 0 a 200: 0rpm=-90°, 200rpm=+90°
            if(angulo<-90) //nunca deberia entrar aca, pero por las dudas: evito que se salga del rango -90 a +90
            {
                angulo=-90;
            }
            else if(angulo>90)
            {
                angulo=90;
            }
            servoWrite(angulo); //actualizo angulo del servo

            num2str(joystickVal, uart_msg); //armo mensaje de uart
            num2str_rpm(rpm, &(uart_msg[3]));
            num2str(angulo, &(uart_msg[6]));

            UARTSend(uart_msg); //envio joystickVal, rpm y angulo por uart

            gpioToggle(RED_LED_MSK); //toggle led para ver que se cumplio un ciclo :)
        }
    }
}
*/
/******************************************************************************/
/*
//Prueba 5.0: pwm con timer1 desde pin 2.1, y rtis con timer0. Cambio system.
//Leo joystick desde pin 1.3.
//Con pwm.h: funca bien.
//Con servo.h: también :)

//Drivers
#include "common.h"
#include "board.h"
#include "system.h"
#include "rti.h"
#include "gpio.h"
#include "joystick_2.h"
#include "servo.h"
//#include "pwm.h"

//Var globales
uint8_t cycle_flag;
//------------------------------------------------------------
//Funciones
void setFlag(void)
{
    cycle_flag=1;
}
//------------------------------------------------------------
void char2str(int8_t num, uint8_t *p2char)
{ //sprintf muy sencillo y ad hoc. Convierto +-90 en '+-90'
    uint8_t i=0;
    uint8_t div=10;
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
//------------------------------------------------------------

//MAIN
void main(void)
{
    uint16_t ciclos_cada_50ms = 1; //quiero hacer un ciclo cada 50ms
    int8_t joystickVal; //valor del joystick que leo
    int8_t angulo = 0; //angulo del servo
    //uint16_t ccr1_pwm = 1000;
    uint8_t timer_servo = 1; //uso el Timer1_A para el pwm del servo

    systemInit(MHZ_1, MS_50, RTI_TIMER0); //stop watchdog, set clock in 1MHz, set rti in 1 interrupt/50ms. Rti's with Timer0_A.
    joystickInit(PORTNUM2PIN(1,3)); //leo el joystick desde P1.3
    servoInit(timer_servo,angulo,P2_1_SERVO); //inicio el servo con el timer1 y el pin 2.1
    //pwmInit(timer_servo, HIGH_LOW_PWM, 20000, ccr1_pwm, P2_1_PWM);

    rtiSubmitCallback(setFlag, ciclos_cada_50ms);
    gpioMode(RED_LED_MSK,OUTPUT);

    enable_interrupts();
    while(TRUE)
    {
        if(cycle_flag)
        {
            cycle_flag=0; //bajo flag
            joystickVal=joystickRead(); //leo joystick (-10 a +10)
            angulo += joystickVal/2; //cada 50ms, sumo valJoystick/2 al angulo
            if(angulo<-90) //evito que se salga del rango -90 a +90
            {
                angulo=-90;
            }
            else if(angulo>90)
            {
                angulo=90;
            }
            servoWrite(angulo); //actualizo angulo del servo
//            if(joystickVal<0) //evito que se salga del rango -90 a +90
//            {
//                ccr1_pwm -= (uint16_t)(-joystickVal)*10;
//                if(ccr1_pwm<500)
//                {
//                    ccr1_pwm=500;
//                }
//            }
//            else if(joystickVal>=0)
//            {
//                ccr1_pwm += (uint16_t)(joystickVal)*10;
//                if(ccr1_pwm>19500)
//                {
//                    ccr1_pwm=19500;
//                }
//            }
//            pwmWrite(timer_servo, 20000, ccr1_pwm);
            gpioToggle(RED_LED_MSK);
        }
    }

}
*/

/******************************************************************************/
/******************************************************************************/
/*        DE ACA PARA ABAJO, FUERON PROBADOS EN Drivers_Tablero_9_11.         */
/******************************************************************************/
/******************************************************************************/

/*
//prueba 4.3: leo joystick periodicamente, sumo su valor al de las rpms, con las rpms calculo el angulo del servo.
//Mientras, envio joystickVal, rpm, angulo por uart, como control.
//NOTA: NO encontre forma facil de enviar el int8_t o uint8_t de joystick, rpm, angulo.
//En cambio, envio ese valor pasado a string ('+07', '-15', etc).
//Con ciclos de 50ms: anda vraiment bien

//Drivers
#include "common.h"
#include "board.h"
#include "system.h"
#include "rti.h"
#include "gpio.h"
#include "comms.h"
#include "joystick_2.h"
#include "servo.h"

//Var globales
uint8_t cycle_flag;
//------------------------------------------------------------
//Funciones
void setFlag(void)
{
    cycle_flag=1;
}
//------------------------------------------------------------
void num2str(int8_t num, uint8_t *p2char)
{ //sprintf muy sencillo y ad hoc. Convierto +-10 en '+-10'
    uint8_t i=0;
    uint8_t div=10;
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
//------------------------------------------------------------
void num2str_rpm(uint8_t num, uint8_t *p2char)
{ //sprintf muy sencillo y ad hoc. Convierto 0 a 200 en '000' a '200'.
    uint8_t i;
    uint8_t div=100;
    for(i=0;i<3;i++)
    {
        *(p2char+i)=(num/div)+'0';
        num -= (num/div)*div;
        div /= 10;
    }
}
//------------------------------------------------------------
int8_t str2num(uint8_t *p2char)
{ //conversion muy sencilla y ad hoc. Convierto '+-90' en +-90
    uint8_t i=2;
    int8_t num=0;
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
//------------------------------------------------------------

//MAIN
void main(void)
{
    uint16_t ciclos_cada_50ms = 1; //quiero hacer un ciclo cada 50ms
    int8_t joystickVal; //valor del joystick que leo y envío.
    int8_t angulo = 0; //ángulo del servo que calculo
    uint8_t rpm = 0; //lectura de rpm's que recibo.
    uint8_t uart_msg[11]; //mensaje que envio por uart para chequear que tengo valores coherentes.
    uint8_t i;

    for(i=0;i<9;i++)
    {
        uart_msg[i]='0';
    }
    uart_msg[9]='\n';
    uart_msg[10]='\0';

    systemInit(MHZ_1, MS_50); //stop watchdog, set clock in 1MHz, set rti in 1 interrupt/ms. Rti's with Timer1_A.
    commsInit(0); //uart_msg_length: no me interesa, le pongo 0.
    joystickInit(ADC_MSK);
    servoInit(0,0,P1_6_SERVO);

    rtiSubmitCallback(setFlag, ciclos_cada_50ms);
    gpioMode(RED_LED_MSK,OUTPUT);
    gpioWrite(RED_LED_MSK,LOW);

    enable_interrupts();
    while(TRUE)
    {
        if(cycle_flag)
        {
            cycle_flag=0; //bajo flag
            joystickVal=joystickRead(); //leo joystick (-10 a +10)
            if(joystickVal>=0)
            {
                rpm += (uint8_t)joystickVal;
                if(rpm>200)
                {
                    rpm=200;
                }
            }
            else //joystickVal<0
            {
                rpm -= (uint8_t)(-joystickVal);
                if(rpm>245) //negative overflow
                {
                    rpm=0;
                }
            }
            angulo = (int8_t)( (int16_t)(  ( (uint16_t)rpm*180 )  /200)   -90); //si las rpm van de 0 a 200: 0rpm=-90°, 200rpm=+90°
            if(angulo<-90) //nunca deberia entrar aca, pero por las dudas: evito que se salga del rango -90 a +90
            {
                angulo=-90;
            }
            else if(angulo>90)
            {
                angulo=90;
            }
            servoWrite(angulo); //actualizo angulo del servo

            num2str(joystickVal, uart_msg); //armo mensaje de uart
            num2str_rpm(rpm, &(uart_msg[3]));
            num2str(angulo, &(uart_msg[6]));

            UARTSend(uart_msg); //envio joystickVal, rpm y angulo por uart

            gpioToggle(RED_LED_MSK); //toggle led para ver que se cumplio un ciclo :)
        }
    }
}
*/
/******************************************************************************/
/*
//prueba 4.2: leo joystick periodicamente, lo envio por uart, espero hasta recibir un valor de angulo por uart,
//y con ese valor que recibo, uso servo.h para mover el servo.
//Con ciclos de 50ms: anda excelenchi

//Drivers
#include "common.h"
#include "board.h"
#include "system.h"
#include "rti.h"
#include "gpio.h"
#include "comms.h"
#include "joystick_2.h"
#include "servo.h"

//Var globales
uint8_t cycle_flag;
//------------------------------------------------------------
//Funciones
void setFlag(void)
{
    cycle_flag=1;
}
//------------------------------------------------------------
void num2str(int8_t num, uint8_t *p2char)
{ //sprintf muy sencillo y ad hoc. Convierto +-10 en '+-10'
    uint8_t i=0;
    uint8_t div=10;
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
//------------------------------------------------------------
int8_t str2num(uint8_t *p2char)
{ //conversion muy sencilla y ad hoc. Convierto '+-90' en +-90
    uint8_t i=2;
    int8_t num=0;
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
//------------------------------------------------------------

//MAIN
void main(void)
{
    uint8_t uart_msg_length = 3; //espero recibir algo como '+75', '-04'.
    uint16_t ciclos_cada_50ms = 1; //quiero hacer un ciclo cada 50ms
    uint8_t uart_msg[5]; //mensaje de uart que quiero mandar
    uint8_t uart_received[5]; //mensaje de uart que quiero recibir
    int8_t joystickVal; //valor del joystick que leo
    int8_t angulo = 0; //angulo del servo

    uart_msg[0]='+';
    uart_msg[1]='0';
    uart_msg[2]='0';
    uart_msg[3]='\n';
    uart_msg[4]='\0';

    uart_received[0]='0';
    uart_received[1]='0';
    uart_received[2]='0';
    uart_received[3]='\n';
    uart_received[4]='\0';

    systemInit(MHZ_1, MS_50); //stop watchdog, set clock in 1MHz, set rti in 1 interrupt/ms. Rti's with Timer1_A.
    commsInit(uart_msg_length);
    joystickInit(ADC_MSK);
    servoInit(0,0,P1_6_SERVO);

    rtiSubmitCallback(setFlag, ciclos_cada_50ms);
    gpioMode(RED_LED_MSK,OUTPUT);
    gpioWrite(RED_LED_MSK,LOW);

    enable_interrupts();
    while(TRUE)
    {
        if(cycle_flag)
        {
            cycle_flag=0; //bajo flag
            joystickVal=joystickRead(); //leo joystick (-10 a +10)
            num2str(joystickVal, uart_msg); //convierto joystickVal en string en uart_msg
            UARTSend(uart_msg); //envio msj
            while(!UARTStatus()){
                //wait
            }
            UARTReceive(uart_received); //recibir msj de uart y guardarlo adonde apunta ese p2char
            angulo = str2num(uart_received); //convertir msj en un numero: el angulo del servo
            if(angulo<-90) //evito que se salga del rango -90 a +90
            {
                angulo=-90;
            }
            else if(angulo>90)
            {
                angulo=90;
            }
            servoWrite(angulo); //actualizo angulo del servo
            num2str(angulo, uart_msg); //convierto angulo en string en uart_msg
            UARTSend(uart_msg); //envio msj
            gpioToggle(RED_LED_MSK); //toggle led para ver que se cumplio un ciclo :)
        }
    }
}
*/
/******************************************************************************/
/*
//prueba 4.1: leo joystick periodicamente, lo envio por uart, espero hasta recibir un valor de angulo por uart,
//y con ese valor que recibo, uso servo.h para mover el servo.
//Con ciclos de 4seg, para que pueda escribir desde serial monitor de arduino, anda joya.
//

//Drivers
#include "common.h"
#include "board.h"
#include "system.h"
#include "rti.h"
#include "gpio.h"
#include "comms.h"
#include "joystick_2.h"
#include "servo.h"

//Var globales
uint8_t cycle_flag;
//------------------------------------------------------------
//Funciones
void setFlag(void)
{
    cycle_flag=1;
}
//------------------------------------------------------------
void num2str(int8_t num, uint8_t *p2char)
{ //sprintf muy sencillo y ad hoc. Convierto +-10 en '+-10'
    uint8_t i=0;
    uint8_t div=10;
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
//------------------------------------------------------------
int8_t str2num(uint8_t *p2char)
{ //conversion muy sencilla y ad hoc. Convierto '+-90' en +-90
    uint8_t i=2;
    int8_t num=0;
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
//------------------------------------------------------------

//MAIN
void main(void)
{
    uint8_t uart_msg_length = 4; //espero recibir algo como '+75\n', '-04\n'.
    uint16_t ciclos_cada_50ms = 80; //quiero hacer un ciclo cada 4s
    uint8_t uart_msg[5]; //mensaje de uart que quiero mandar
    uint8_t uart_received[5]; //mensaje de uart que quiero recibir
    int8_t joystickVal; //valor del joystick que leo
    int8_t angulo = 0; //angulo del servo

    uart_msg[0]='+';
    uart_msg[1]='0';
    uart_msg[2]='0';
    uart_msg[3]='\n';
    uart_msg[4]='\0';

    uart_received[0]='0';
    uart_received[1]='0';
    uart_received[2]='0';
    uart_received[3]='\n';
    uart_received[4]='\0';

    systemInit(MHZ_1, MS_50); //stop watchdog, set clock in 1MHz, set rti in 1 interrupt/ms. Rti's with Timer1_A.
    commsInit(uart_msg_length);
    joystickInit(ADC_MSK);
    servoInit(0,0,P1_6_SERVO);

    rtiSubmitCallback(setFlag, ciclos_cada_50ms);
    gpioMode(RED_LED_MSK,OUTPUT);
    gpioWrite(RED_LED_MSK,LOW);

    enable_interrupts();
    while(TRUE)
    {
        if(cycle_flag)
        {
            cycle_flag=0; //bajo flag
            joystickVal=joystickRead(); //leo joystick (-10 a +10)
            num2str(joystickVal, uart_msg); //convierto joystickVal en string en uart_msg
            UARTSend(uart_msg); //envio msj
            while(!UARTStatus()){
                //wait
            }
            UARTReceive(uart_received); //recibir msj de uart y guardarlo adonde apunta ese p2char
            angulo = str2num(uart_received); //convertir msj en un numero: el angulo del servo
            if(angulo<-90) //evito que se salga del rango -90 a +90
            {
                angulo=-90;
            }
            else if(angulo>90)
            {
                angulo=90;
            }
            servoWrite(angulo); //actualizo angulo del servo
            num2str(angulo, uart_msg); //convierto angulo en string en uart_msg
            UARTSend(uart_msg); //envio msj
            gpioToggle(RED_LED_MSK); //toggle led para ver que se cumplio un ciclo :)
        }
    }

}
*/
/******************************************************************************/
/*
//prueba 3.2: leo joystick periodicamente, uso ese valor y servo.h para mover el servo.
//envio dicho valor por uart.
//anda 10 puntos

//Drivers
#include "common.h"
#include "board.h"
#include "system.h"
#include "rti.h"
#include "gpio.h"
#include "comms.h"
#include "joystick_2.h"
#include "servo.h"

//Var globales
uint8_t cycle_flag;
//------------------------------------------------------------
//Funciones
void setFlag(void)
{
    cycle_flag=1;
}
//------------------------------------------------------------
void char2str(int8_t num, uint8_t *p2char)
{ //sprintf muy sencillo y ad hoc. Convierto +-90 en '+-90'
    uint8_t i=0;
    uint8_t div=10;
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
//------------------------------------------------------------

//MAIN
void main(void)
{
    uint8_t uart_msg_length = 5; //ponele
    uint16_t ciclos_cada_50ms = 1; //quiero hacer un ciclo cada 50ms
    uint8_t uart_msg[5]; //mensaje de uart que quiero mandar
    int8_t joystickVal; //valor del joystick que leo
    int8_t angulo = 0; //angulo del servo

    uart_msg[0]='+';
    uart_msg[1]='0';
    uart_msg[2]='0';
    uart_msg[3]='\n';
    uart_msg[4]='\0';

    systemInit(MHZ_1, MS_50); //stop watchdog, set clock in 1MHz, set rti in 1 interrupt/ms. Rti's with Timer1_A.
    commsInit(uart_msg_length);
    joystickInit(ADC_MSK);
    servoInit(0,0,P1_6_SERVO);

    rtiSubmitCallback(setFlag, ciclos_cada_50ms);
    gpioMode(RED_LED_MSK,OUTPUT);

    enable_interrupts();
    while(TRUE)
    {
        if(cycle_flag)
        {
            cycle_flag=0; //bajo flag
            joystickVal=joystickRead(); //leo joystick (-10 a +10)
            angulo += joystickVal/2; //cada 50ms, sumo valJoystick/2 al angulo
            if(angulo<-90) //evito que se salga del rango -90 a +90
            {
                angulo=-90;
            }
            else if(angulo>90)
            {
                angulo=90;
            }
            servoWrite(angulo); //actualizo angulo del servo
            char2str(angulo, uart_msg); //convierto angulo a string en uart_msg
            UARTSend(uart_msg); //envio msj
            gpioToggle(RED_LED_MSK);
        }
    }

}
*/
/******************************************************************************/
/*
//prueba 3.1: leo joystick periodicamente, sumo su valor al ccr1 del pwm de un led.
//envio dicho valor por uart.

//Drivers
#include "common.h"
#include "board.h"
#include "system.h"
#include "rti.h"
#include "gpio.h"
#include "comms.h"
#include "joystick_2.h"
#include "pwm.h"

//Var globales
uint8_t cycle_flag;
//------------------------------------------------------------
//Funciones
void setFlag(void)
{
    cycle_flag=1;
}
//------------------------------------------------------------
void char2str(uint16_t num, uint8_t *p2char)
{ //sprintf muy sencillo y ad hoc. Convierto 0 a 9999 en '0000', '9999'
    uint8_t i;
    uint16_t div=1000;
    for(i=0;i<4;i++)
    {
        *(p2char+i)=(num/div)+'0';
        num = num - (num/div)*div;
        div /= 10;
    }
}
//------------------------------------------------------------

//MAIN
void main(void)
{
    uint8_t uart_msg_length = 5; //ponele
    uint16_t ciclos_cada_50ms = 1; //quiero hacer un ciclo cada 50ms
    uint8_t uart_msg[6]; //mensaje de uart que quiero mandar
    int8_t joystickVal; //valor del joystick que leo
    uint16_t pwm_ccr1=1000; //valor de ccr1 del pwm en us (quiero que vaya de 500 a 2500)

    uart_msg[0]='1';
    uart_msg[1]='0';
    uart_msg[2]='0';
    uart_msg[3]='0';
    uart_msg[4]='\n';
    uart_msg[5]='\0';

    systemInit(MHZ_1, MS_50); //stop watchdog, set clock in 1MHz, set rti in 1 interrupt/ms. Rti's with Timer1_A.
    commsInit(uart_msg_length);
    joystickInit(ADC_MSK);
    pwmInit(0, HIGH_LOW_PWM, 20000, pwm_ccr1, P1_6_PWM);

    rtiSubmitCallback(setFlag, ciclos_cada_50ms);
    gpioMode(RED_LED_MSK,OUTPUT);

    enable_interrupts();
    while(TRUE)
    {
        if(cycle_flag)
        {
            cycle_flag=0; //bajo flag
            joystickVal=joystickRead(); //leo joystick (-10 a +10)
            pwm_ccr1 += joystickVal/2; //cada 50ms, sumo valJoystick/2 al ccr1
            pwmWrite(0, 20000, pwm_ccr1); //actualizo ccr1 del pwm
            char2str(pwm_ccr1, uart_msg); //convierto ccr1 a string en uart_msg
            UARTSend(uart_msg); //envio msj
            gpioToggle(RED_LED_MSK);
        }
    }

}
*/
/******************************************************************************/
/*
//prueba 2.2: leo joystick periodicamente, envio su valor a la pc por uart y veo si me escucha.
//Uso joystick_2 y adc_2. Tb anda barbaro.

//Drivers
#include "common.h"
#include "board.h"
#include "system.h"
#include "rti.h"
#include "gpio.h"
#include "comms.h"
#include "joystick_2.h"

//Var globales
uint8_t send_flag;
//------------------------------------------------------------
//Funciones
void setFlag(void)
{
    send_flag=1;
}
//------------------------------------------------------------
void char2str(int8_t num, uint8_t *p2char)
{ //sprintf muy sencillo y ad hoc. Convierto +-10 en '+-10'
    uint8_t i=0;
    if(num<0)
    {
        *(p2char+i)='-';
    }
    else
    {
        *(p2char+i)='+';
    }
    i++;
    if(num==10 | num==-10)
    {
        *(p2char+i)='1';
        i++;
        *(p2char+i)='0';
    }
    else
    {
        *(p2char+i)='0';
        i++;
        if(num>=0)
        {
            *(p2char+i)=num+'0';
        }
        else
        {
            *(p2char+i)=-num+'0';
        }
    }
}
//------------------------------------------------------------

//MAIN
void main(void)
{
    uint8_t uart_msg_length = 5; //ponele
    uint16_t ciclos_cada_50ms = 1; //quiero mandar un msj por uart cada 50ms
    uint8_t uart_msg[5]; //mensaje de uart que quiero mandar
    int8_t joystickVal;

    uart_msg[0]='+';
    uart_msg[1]='0';
    uart_msg[2]='0';
    uart_msg[3]='\n';
    uart_msg[4]='\0';

    systemInit(MHZ_1, MS_50); //stop watchdog, set clock in 1MHz, set rti in 1 interrupt/50ms. Rti's with Timer1_A.
    commsInit(uart_msg_length);
    joystickInit(ADC_MSK);
    rtiSubmitCallback(setFlag, ciclos_cada_50ms); //un ciclo cada 50ms
    gpioMode(RED_LED_MSK,OUTPUT);

    enable_interrupts();
    while(TRUE)
    {
        if(send_flag)
        {
            send_flag=0; //bajo flag
            joystickVal=joystickRead(); //leo joystick (-10 a +10)
            char2str(joystickVal, uart_msg); //lo convierto a string en uart_msg
            UARTSend(uart_msg); //envio msj
            gpioToggle(RED_LED_MSK);
        }
    }

}
*/
/******************************************************************************/
/*
//prueba 2.1: leo joystick periodicamente, envio su valor a la pc por uart y veo si me escucha.
//Uso joystick y adc. Anda barbaro

//Drivers
#include "common.h"
#include "board.h"
#include "system.h"
#include "rti.h"
#include "gpio.h"
#include "comms.h"
#include "joystick.h"

//Var globales
uint8_t send_flag;
//------------------------------------------------------------
//Funciones
void setFlag(void)
{
    send_flag=1;
}
//------------------------------------------------------------
void char2str(int8_t num, uint8_t *p2char)
{ //sprintf muy sencillo y ad hoc. Convierto +-10 en '+-10'
    uint8_t i=0;
    if(num<0)
    {
        *(p2char+i)='-';
    }
    else
    {
        *(p2char+i)='+';
    }
    i++;
    if(num==10 | num==-10)
    {
        *(p2char+i)='1';
        i++;
        *(p2char+i)='0';
    }
    else
    {
        *(p2char+i)='0';
        i++;
        if(num>=0)
        {
            *(p2char+i)=num+'0';
        }
        else
        {
            *(p2char+i)=-num+'0';
        }
    }
}
//------------------------------------------------------------

//MAIN
void main(void)
{
    uint8_t uart_msg_length = 5; //ponele
    uint16_t periodo_de_msg_ms = 500; //quiero mandar un msj por uart cada 500ms
    uint8_t uart_msg[5]; //mensaje de uart que quiero mandar
    int8_t joystickVal;

    uart_msg[0]='+';
    uart_msg[1]='0';
    uart_msg[2]='0';
    uart_msg[3]='\n';
    uart_msg[4]='\0';

    systemInit(MHZ_1, MS_1); //stop watchdog, set clock in 1MHz, set rti in 1 interrupt/ms. Rti's with Timer1_A.
    commsInit(uart_msg_length);
    joystickInit(ADC_MSK);
    rtiSubmitCallback(setFlag, periodo_de_msg_ms);
    gpioMode(RED_LED_MSK,OUTPUT);

    enable_interrupts();
    while(TRUE)
    {
        if(send_flag)
        {
            send_flag=0; //bajo flag
            joystickVal=joystickRead(); //leo joystick (-10 a +10)
            char2str(joystickVal, uart_msg); //lo convierto a string en uart_msg
            UARTSend(uart_msg); //envio msj
            gpioToggle(RED_LED_MSK);
        }
    }

}
*/
/******************************************************************************/
/*
//prueba 1: envio numeros a la pc por uart y veo si me escucha
//anda joya

//Drivers
#include "common.h"
#include "board.h"
#include "system.h"
#include "rti.h"
#include "gpio.h"
#include "comms.h"
//#include "uart.h"
//#include "hw_timer.h"

//Var globales
uint8_t send_flag;

//Funciones
void setFlag(void)
{
    send_flag=1;
}

//MAIN
void main(void)
{


    uint8_t uart_msg_length = 5; //ponele
    uint16_t periodo_de_msg_ms = 500; //quiero mandar un msj por uart cada 500ms
    uint8_t uart_msg[8]; //mensaje de uart que quiero mandar
    uint16_t counter[2]; //un contador para ir modificando el msj
    uint8_t i;

    for(i=0;i<6;++i)
    {
        uart_msg[i]='0';
    }
    uart_msg[6]='\n';
    uart_msg[7]=0;

    counter[0]=0; counter[1]=0;

    systemInit(MHZ_1, MS_1); //stop watchdog, set clock in 1MHz, set rti in 1 interrupt/ms. Rti's with Timer1_A.
    commsInit(uart_msg_length);
    //uartInit(uart_msg_length);
    rtiSubmitCallback(setFlag, periodo_de_msg_ms);
    //hwTimerInit(0); //inicializar hwTimer-->Timer0_A
    //hwSetTimer(0, OUTPUT_COMPARE_HWT, UP_MODE_HWT, OUTPUT_MODE_HWT, 0, 50000, 0, *setFlag, NULL);
    gpioMode(RED_LED_MSK,OUTPUT);

    enable_interrupts();
    while(TRUE)
    {
        if(send_flag)
        {
            send_flag=0; //bajo flag
            UARTSend(uart_msg); //envio msj
            //uartWriteString(uart_msg);
            //uartSend();
            gpioToggle(RED_LED_MSK);

            //toqueteo el msj:
            uart_msg[counter[1]]++;
            counter[0]++;
            if(counter[0]>8)
            {
                counter[0]=0;
                counter[1]++;
            }
            if(counter[1]>5)
            {
                counter[1]=0;
                for(i=0;i<6;++i)
                {
                    uart_msg[i]='0';
                }
            }
            //end toqueteo el msj

        }
    }

}
*/
/******************************************************************************/
