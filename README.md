# MSP430 Motor ECU

### Español
Este repo contiene la integración de dos microcontroladores MSP430 con:
- un motor DC
- un motor Servo
- un optoacoplador de ranura
- un joystick
- un led
- dos módulos CAN (MCP2515+TJA1050)
- una PC Windows con Matlab

El conjunto controla la velocidad del motor DC con el Joystick o por computadora,
leyéndola con el optoacoplador de ranura actuando como un encoder, y
mostrándola en una interfaz gráfica en Matlab y también con el motor Servo usado como velocímetro.
A su vez, la lampara led actúa como bujía, encendiéndose en cada vuelta entre determinados ángulos.

La comunicación entre los dos MSP430 ("Tablero" y "Motor") se realiza por protocolo CAN,
y la comunicación de uno de ellos ("Tablero") con la PC se realiza por UART.

El proyecto fue realizado mayormente en C, y en parte en Matlab, entre julio y diciembre de 2022,
como trabajo práctico final de la materia Microprocesadores y Control de la carrera de Ingeniería
Mecánica en el Instituto Tecnológico de Buenos Aires (ITBA), junto con los Ing. Jorge P. Torres,
Marcos Olivera, Juan José Zannier Díaz y Nicolás Capparelli.

Cuenta con un informe, las consignas/alcance del proyecto, una carpeta con el código para el MSP
que actúa como "Motor", otra para el que actúa como "Tablero", otra con el código de Matlab para
la PC, y otra adicional con algunas mejores versiones de los drivers .c y .h usados en el proyecto.

A continuación, algunas imágenes del hardware (y el software de visualización):

![image](https://github.com/JuanMartinMujicaBuj/MSP430_motor_ECU/assets/53155661/f28faea9-3114-411c-a7d6-96c95ca604ca)
![image](https://github.com/JuanMartinMujicaBuj/MSP430_motor_ECU/assets/53155661/02e9c612-d347-40b7-aea3-c59cb72b3d67)
![image](https://github.com/JuanMartinMujicaBuj/MSP430_motor_ECU/assets/53155661/05cdffef-4f26-4bd1-9c27-1c28924cf1c7)
![image](https://github.com/JuanMartinMujicaBuj/MSP430_motor_ECU/assets/53155661/24e47b69-98c7-4223-abb5-53c29f75325c)
![image](https://github.com/JuanMartinMujicaBuj/MSP430_motor_ECU/assets/53155661/e73f34e9-fa76-43ad-9d8a-62a6261e62df)

### English
This repo contains the integration of two MSP430 microcontrollers with:
- a DC motor
- a Servo Motor
- a slotter optocoupler
- a joystick
- a led
- two CAN modules (MCP2515+TJA1050)
- a Windows PC with Matlab

The assembly controls the DC motor speed via the Joystick or the computer, it measures it
with the slotted optocoupler acting as an encoder, and it displays it in a graphical user
interface on Matlab, as well as with the Servo motor which acts as a speedometer.
Meanwhile, the led simulates a spark plug that is switched on and off at certain angles.

The comms between both MSP430 ("Control Panel" and "Motor") is done under CAN protocol, while
the comms between the "Control Panel" and the computer is done with UART.

Most of the project was done in C, and the remaining part of it in Matlab, between July and
December 2022, as a final project for the Microprocessors and Control course of the Mechanical
Engineering degree at the Buenos Aires Institute of Technology (ITBA), together with Eng. Jorge
P. Torres, Marcos Olivera, Juan José Zannier Díaz and Nicolás Capparelli.

It comprises a report, the expected task/results, a folder with the "Control Panel" MSP code,
another one with the "Motor" MSP code, another one with the Matlab code for the computer, and
a separate one with better versions of the .c and .h driver files.

Some valuable pictures of the hardware and the visualization software can be seen above.
