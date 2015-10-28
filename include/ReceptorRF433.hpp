/*
    La clase ReceptorRF433 recibe los mensajes del modulo RF-Receiver conectado a la entrada
    RF_PIN de las gpios de la Raspberry.
    Los mensajes tienen 24 bits de longitud, los dos bytes menos significativos contienen el
    cuerpo del mensaje. El byte siguiente contiene un n�mero de 1 a N_MENSAJES indicando el
    tipo de mensaje.
    Los clientes deben llamar a la funci�n arranca() para iniciar la recepci�n.
    Los mensajes se depositan en mensaje_tipo[] seg�n su tipo. mensaje_tipo[] contiene el �ltimo
    mensaje recibido de cada tipo.

    ReceptorRF433 es una clase static pura y no se necesita ninguna instanciaci�n.

    Se usa parte del c�digo de la clase RCSwitch:
            RCSwitch - Arduino libary for remote control outlet switches
            Copyright (c) 2011 Suat �zg�r.  All right reserved.

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
    static const int N_MENSAJES = 6;   // N�mero total de mensajes

//  static unsigned long nReceivedValue;        // Aqui est� el mensaje recibido o es 0
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
