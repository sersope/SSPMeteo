#ifndef __RECEPTOR__RF__433_HPP
#define __RECEPTOR__RF__433_HPP

#include <wiringPi.h>

/** \brief Clase para la recepción de mensajes desde la estación.
 *
 *  La clase ReceptorRF433 recibe los mensajes del modulo RF-Receiver conectado a la entrada
 *  RF_PIN de la GPIO de la Raspberry.
 *  Los mensajes tienen 24 bits de longitud, los dos bytes menos significativos contienen el
 *  cuerpo del mensaje. El byte siguiente contiene un número de 1 a N_MENSAJES indicando el
 *  tipo de mensaje.
 *  Cada mensaje tipo contiene un tipo de información determinada de la estación (temperatura,humedad,...)
 *  La estación realiza una emisión de mensajes periodicamente cada 50 segundos.
 *  Cada tipo de mensaje se recibe un número determinado de veces (REPETIDOS) para
 *  asegurar la fiabilida de los datos y la calidad de la transmisión.
 *  Los mensajes se depositan en el array doble mensaje_tipo. mensaje_tipo contiene todos
 *  los mensajes recibidos de cada tipo en la última transmisión.
 *
 *  ReceptorRF433 es una clase static pura y por lo tanto no se necesita ninguna instanciación.
 *  En el inicio se debe llamar a la función arranca() para iniciar la recepción.
 *
 *  Para el acceso a la GPIO se utiliza la librería WiringPi (http://wiringpi.com/).
 *
 *  Se usa parte del código de la clase RCSwitch:
 *          - RCSwitch - Arduino libary for remote control outlet switches.
 *          - Copyright (c) 2011 Suat Özgür.  All right reserved.
 *          - Contributors:
 *          -   Andre Koehler / info(at)tomate-online(dot)de
 *          -   Gordeev Andrey Vladimirovich / gordeev(at)openpyro(dot)com
 *          -   Skineffect / http://forum.ardumote.com/viewtopic.php?f=2&t=46
 *          -   Dominik Fischer / dom_fischer(at)web(dot)de
 *          -   Frank Oltmanns / <first name>.<last name>(at)gmail(dot)com
 *          - Project home: http://code.google.com/p/rc-switch/
*/
class ReceptorRF433
{
  private:
    static const int RECEIVE_TOLERANCE = 60;
    static const int PACKET_MAX_CHANGES = 67;
    static const int RF_PIN = 2;        // Pin gpio para la recepción de mensajes.
    static const int N_MENSAJES = 6;    // Número de mensajes tipo
    static const int REPETIDOS = 9;     //  Repeticon de cada mensaje tipo
    static unsigned int timings[PACKET_MAX_CHANGES];
    static int last_mensa;
    static void handleInterrupt();
    static bool receiveProtocol1(unsigned int changeCount);
    static void reseteaArraysMensajes();
  public:
    /// Contiene la cantidad de mensajes recibidos de cada tipo en la última transmisión.
    static int mensaje_indice[N_MENSAJES];
    /// Contiene el nº total de mensajes recibidos en la última transmisión.
    static int mensajes_recibidos;
    ///Contiene todos los mensajes recibidos en la última transmisión.
    static unsigned mensaje_tipo[N_MENSAJES][REPETIDOS];
    /// Habilita la recepción de mensajes.
    static bool arranca();
    /// Comprueba si el mensaje recibido es válido
    static bool mensajeOK(int nmen);
};
#endif //__RECEPTOR__RF__433_HPP
