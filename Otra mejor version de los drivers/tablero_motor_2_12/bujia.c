/***************************************************************************//**
  @file     bujia.c
  @brief    Spark plug (bujía) services. Switch on or off according to motor speed
            and start and stop angles.
  @authors  Mujica Buj
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include "bujia.h"
#include "rti.h"
#include "gpio.h"

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/
#define RTI_INTERVAL 1 //nro de interrupts cada las que debe llamarse la
                       //funcion de interrupcion periodica
#define PERIODO 1/2 //tiempo entre interrupts, en ms.

/*******************************************************************************
 * STATIC VARIABLES AND CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/
static uint8_t callback_id; //id del callback de rti_bujia
static uint8_t rpm;         //rpm del motor. Buffer interno del driver de bujia.
static uint16_t phi1_x100;  //angulo de encendido de la bujia, x100. Buffer interno del driver.
static uint16_t phi2_x100;  //angulo de apagado de la bujia, x100. Buffer interno del driver.
static uint16_t phi_x100;   //angulo actual del motor, multiplicado por 100 (para trabajar
                            //en ints y tener definicion de una centesima de grado).
static uint8_t pin;         //pin de salida para encender o apagar la bujia.
static uint8_t yaInit;      //para no inicializar dos veces el driver.
static uint8_t noInterrupt; //para evitar actualizar angulo, encender o apagar bujia
                            //mientras modifico los valores de rpm, phi1, phi2
static uint16_t delta_phi_x100; //avance del angulo phi a sumar en cada interrupt.

/*******************************************************************************
 * PRIVATE FUNCTION PROTOTYPES
 ******************************************************************************/
void rti_bujia(void); //actualizar angulo calculado del motor y estado de la bujia

/*******************************************************************************
* GLOBAL FUNCTION DEFINITIONS
*******************************************************************************/
// Inicializar driver de bujía. Guardar velocidad de giro y angulo de encendido
// y apagado iniciales. Suscribirse a interrupts periodicas, en las que actualizar
// el angulo y el estado (on-off) de la salida (la bujia).
void bujiaInit(uint8_t pin_in, uint8_t rpm_in, uint16_t phi1_in, uint16_t phi2_in)
{
    if (yaInit)
        return;
    yaInit = 1;

    rpm = rpm_in; //al ser uint8_t: vel max=255rpm (no necesito mas que eso)
    phi1_x100 = phi1_in*100;
    phi2_x100 = phi2_in*100;

    if(phi1_x100>35800)
    {
        phi1_x100 = 35800; //no puede ser mayor que 358°
    }
    if(phi2_x100>35900)
    {
        phi2_x100 = 35900; //no puede ser mayor que 359°
    }
    if(phi2_x100<=phi1_x100)
    {
        phi2_x100 = phi1_x100+100; //phi2 debe ser mayor que phi1
    }

    //angulo avanzado = rps*360°*segs = (rpm/60)*360*(PERIODO/1000) = {rpm*[(360/60)*PERIODO]}/1000
    delta_phi_x100 = ( ((uint16_t)rpm) * (6*PERIODO) )/10;

    pin=pin_in;
    gpioMode(pin,OUTPUT); //set pin as output

    callback_id=rtiSubmitCallback(rti_bujia, RTI_INTERVAL); //submit callback function to
                                                            //the periodic interrupt module
}

/******************************************************************************/
//Modificar los rpm, phi1 y/o phi2
void bujiaWrite(uint8_t rpm_in, uint16_t phi1_in, uint16_t phi2_in)
{
    noInterrupt=1; //no enciendo/apago bujia mientras modifico (rpm,) phi1, phi2.
                   //Por si en la mitad de la asignacion phi1=phi1_in, entro en la interrupt.

    rpm = rpm_in; //al ser uint8_t: vel max=255rpm (no necesito mas que eso)
    phi1_x100 = phi1_in*100;
    phi2_x100 = phi2_in*100;

    if(phi1_x100>35800)
    {
        phi1_x100 = 35800; //no puede ser mayor que 358°
    }
    if(phi2_x100>35900)
    {
        phi2_x100 = 35900; //no puede ser mayor que 359°
    }
    if(phi2_x100<=phi1_x100)
    {
        phi2_x100 = phi1_x100+100; //phi2 debe ser mayor que phi1
    }

    //aca no se si deberia ir un disable_interrupts(), enable_interrupts() para hacer atomic a la operacion de abajo.
    delta_phi_x100 = ( ((uint16_t)rpm) * (6*PERIODO) )/10;

    noInterrupt=0; //termine de actualizar rpm, phi1, phi2, ya puedo realizar la rutina de rti_bujia
}

/******************************************************************************/
// Cerar (poner en 0) el ángulo de giro (el que se actualiza en las interrupts
// periodicas y se usa para decidir si corresponde prender o apagar la bujia)
void bujiaZero(void)
{
    phi_x100=0;
}


/******************************************************************************/
// Poner en cierto valor el ángulo de giro (el que se actualiza en las interrupts
// periodicas y se usa para decidir si corresponde prender o apagar la bujia).
// Util para ajustar cada cierto tiempo la posicion del angulo, que puede ir desviandose.
void bujiaSetAngle(uint16_t phi_in)
{
    phi_x100=phi_in*100;
}

/******************************************************************************/
//Desactivar la bujia
void bujiaStop(void)
{
    rtiClearCallback(callback_id); //desuscribirse de los interrupts periodicos (no esta demasiado chequeado)
    yaInit=0;
}

/*******************************************************************************
 * PRIVATE FUNCTION DEFINITIONS
 ******************************************************************************/
//Update phi. If phi1<=phi<=phi2, prender bujia, si no, no.
//Si esta activado el flag noInterrupt, solo actualizo phi, con el rpm viejo.
//Si no esta activado, actualizo el rpm y defino si encender o no la bujia.

//Usar con interrupts cada 0.5ms! O modificar el #define PERIODO (1/2)
void rti_bujia(void)
{
    //angulo avanzado = rps*360°*segs = (rpm/60)*360*(PERIODO/1000) = {rpm*[(360/60)*PERIODO]}/1000
    phi_x100 += delta_phi_x100;

    if(phi_x100>=36000) //si me paso de 360°, vuelvo a 0°
    {
        phi_x100 = phi_x100-36000;
    }

    if(!noInterrupt) //si no estoy en el medio de una modificacion de phi1, phi2
    {
        if( (phi_x100>=phi1_x100) & (phi_x100<=phi2_x100) )
        {
            gpioWrite(pin,HIGH); //si phi1<=phi<=phi2, prendo la bujia
        }
        else
        {
            gpioWrite(pin,LOW); //si no, apago la bujia
        }
    }
}
