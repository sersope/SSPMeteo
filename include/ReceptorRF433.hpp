/*
    La clase ReceptorRF433 recibe los mensajes del modulo RF-Receiver conectado a la entrada
    RF_PIN de las gpios de la Raspberry.
    Los mensajes tienen 24 bits de longitud, los dos bytes menos significativos contienen el
    cuerpo del mensaje. El byte siguiente contiene un número de 1 a N_MENSAJES indicando el
    tipo de mensaje.
    Los clientes deben llamar a la función arranca() para iniciar la recepción.
    Los mensajes se depositan en mensaje_tipo[] según su tipo. mensaje_tipo[] contiene el último
    mensaje recibido de cada tipo.

    ReceptorRF433 es una clase static pura y no se necesita ninguna instanciación.

    Se usa parte del código de la clase RCSwitch:
            RCSwitch - Arduino libary for remote control outlet switches
            Copyright (c) 2011 Suat Özgür.  All right reserved.

            Contributors:
            - Andre Koehler / info(at)tomate-online(dot)de
            - Gordeev Andrey Vladimirovich / gordeev(at)openpyro(dot)com
            - Skineffect / http://forum.ardumote.com/viewtopic.php?f=2&t=46
            - Dominik Fischer / dom_fischer(at)web(dot)de
            - Frank Oltmanns / <first name>.<last name>(at)gmail(dot)com

            Project home: http://code.google.com/p/rc-switch/
*/
#ifndef __RECEPTOR__RF__433_HPP
#define __RECEPTOR__RF__433_HPP

#include <wiringPi.h>
//#include <stdint.h>

class ReceptorRF433
{
  private:
	static const int RECEIVE_TOLERANCE = 60;
	static const int PACKET_MAX_CHANGES = 67;
    static const int RF_PIN = 2;
    static const int N_MENSAJES = 6;   // Número total de mensajes

//  static unsigned long nReceivedValue;        // Aqui está el mensaje recibido o es 0
//    static unsigned int nReceivedBitlength;     // Valor solo informativo
//	static unsigned int nReceivedDelay;         // Valor solo informativo
//	static unsigned int nReceivedProtocol;      // Valor solo informativo
    static unsigned int timings[PACKET_MAX_CHANGES];

    static void handleInterrupt();
	static bool receiveProtocol1(unsigned int changeCount);

  public:
    static unsigned mensaje_tipo[N_MENSAJES];
    static bool arranca();

};
#endif //__RECEPTOR__RF__433_HPP
