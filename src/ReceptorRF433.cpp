#include "ReceptorRF433.hpp"
#include <iostream>

unsigned int ReceptorRF433::timings[ReceptorRF433::PACKET_MAX_CHANGES]{};
unsigned int ReceptorRF433::mensaje_tipo[ReceptorRF433::N_MENSAJES][REPETIDOS]{};
int ReceptorRF433::last_mensa = 0;
int ReceptorRF433::mensajes_recibidos = 0;
int ReceptorRF433::mensaje_indice[ReceptorRF433::N_MENSAJES]{};

/** Esta función habilita la recepción de mensajes
 *  desde la estación.
 *  Se inicializa la librería WiringPi.
 *  Se asocia la función manejadora de la interrupción de disparo del pin.
 *
 * \return bool. false si falla la inicializacion de WiringPi, true si OK.
 *
 */
bool ReceptorRF433::arranca()
{
    if(wiringPiSetup() == -1)
        return false;
    else
    {
        wiringPiISR(RF_PIN, INT_EDGE_BOTH, &handleInterrupt);
        reseteaArraysMensajes();
        return true;
    }
}

void ReceptorRF433::handleInterrupt()
{
    static unsigned int duration;
    static unsigned int changeCount;
    static unsigned long lastTime;
    static unsigned int repeatCount;

    long time = micros();
    duration = time - lastTime;

    if (duration > 5000 && duration > timings[0] - 200 && duration < timings[0] + 200)
    {
        repeatCount++;
        changeCount--;

        if (repeatCount == 2)
        {
            if (receiveProtocol1(changeCount) == false)
            {
                //failed
            }
            repeatCount = 0;
        }
        changeCount = 0;
    }
    else if (duration > 5000)
    {
        changeCount = 0;
    }

    if (changeCount >= PACKET_MAX_CHANGES)
    {
        changeCount = 0;
        repeatCount = 0;
    }
    timings[changeCount++] = duration;
    lastTime = time;
}

bool ReceptorRF433::receiveProtocol1(unsigned int changeCount)
{
    unsigned long code = 0;
    unsigned long delay = timings[0] / 31;
    unsigned long delayTolerance = delay * RECEIVE_TOLERANCE * 0.01;

    for (unsigned i = 1; i<changeCount ; i=i+2)
    {
        if (timings[i] > delay-delayTolerance && timings[i] < delay+delayTolerance && timings[i+1] > delay*3-delayTolerance && timings[i+1] < delay*3+delayTolerance)
        {
            code = code << 1;
        }
        else if (timings[i] > delay*3-delayTolerance && timings[i] < delay*3+delayTolerance && timings[i+1] > delay-delayTolerance && timings[i+1] < delay+delayTolerance)
        {
            code+=1;
            code = code << 1;
        }
        else
        {
            // Failed
            i = changeCount;
            code = 0;
        }
    }
    code = code >> 1;
    if (changeCount/2 == 24)      // considera solo mensajes de 24 bits (1er. filtro )
    {
        // Cada tipo de mensaje (de 1 a "N_MENSAJES") se recibe un nº de veces.
        // Los mensajes se almacenan en el array de 2 dimensiones "mensaje_tipo" ordenados por el tipo de mensaje
        // El nº de veces que se recibe cada tipo de mensaje se almacena en el array "mensaje_indice"
        int n = (code & 0xFF0000) >> 16; //tipo de mensaje
        if (n >= 1 && n <= N_MENSAJES) // 2º filtro
        {
            int m = n-1; //Es el indice para los arrays;

            // Comienzo de una nueva transmision. Suponemos siempre por lo menos un mensaje 1 valido :)
            // Si no llega ningun mensaje 1 reseteamos cuando se alcance el máximo nº de mensajes esperados.
            if( (m == 0 && m != last_mensa) || mensajes_recibidos >= (N_MENSAJES * REPETIDOS) )
                reseteaArraysMensajes();

            if( mensaje_indice[m] >= REPETIDOS )
                mensaje_indice[m] = REPETIDOS - 1; // Evita overflow del array
            mensaje_tipo[m][mensaje_indice[m]] = code;
            mensaje_indice[m]++;
            mensajes_recibidos++;
            last_mensa = m;
        }
    }
    if (code == 0)
        return false;
    else
        return true;
}

void ReceptorRF433::reseteaArraysMensajes()
{
    int fila, columna;

    for(fila = 0; fila < N_MENSAJES; fila++)
    {
        mensaje_indice[fila] = 0;
        for(columna = 0; columna < REPETIDOS ; columna++)
            mensaje_tipo[fila][columna] = 99999; // Para distinguir si el mensaje se ocupa
    }
    mensajes_recibidos = 0;
}

/** Si se reciben tres o mas mensajes iguales del mismo tipo
 *  el mensaje se da como bueno.
 *
 * \param nmen Tipo de mensaje
 * \return bool Devuelve true si el mensaje se da como válido.
 *
 */
bool ReceptorRF433::mensajeOK(int nmen)
{
    int n = mensaje_indice[nmen];
    if( n < 3)
        return false;
    unsigned valor = mensaje_tipo[nmen][0];
    for(int i = 0; i < n; i++)
        if( mensaje_tipo[nmen][i] != valor)
            return false;
    return true;
}
