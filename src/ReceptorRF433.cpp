#include "ReceptorRF433.hpp"
#include <iostream>
//unsigned long ReceptorRF433::nReceivedValue = 0;
//unsigned int ReceptorRF433::nReceivedBitlength = 0;
//unsigned int ReceptorRF433::nReceivedDelay = 0;
//unsigned int ReceptorRF433::nReceivedProtocol = 0;
unsigned int ReceptorRF433::timings[ReceptorRF433::PACKET_MAX_CHANGES]{};
unsigned int ReceptorRF433::mensaje_tipo[ReceptorRF433::N_MENSAJES]{};

bool ReceptorRF433::arranca()
{
    if(wiringPiSetup() == -1)
        return false;
    else
    {
        wiringPiISR(RF_PIN, INT_EDGE_BOTH, &handleInterrupt);
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
    if (changeCount/2 == 24)      // considera solo mensajes de 24 bits
    {
//        ReceptorRF433::nReceivedValue = code;
//        ReceptorRF433::nReceivedBitlength = changeCount / 2;
//        ReceptorRF433::nReceivedDelay = delay;
//        ReceptorRF433::nReceivedProtocol = 1;
        // Según el tipo de mensaje asigna al array de mensajes.
        int n = (code & 0xFF0000) >> 16; //Nº del mensaje
        if (n >= 1 && n <= N_MENSAJES)
        {
           mensaje_tipo[n-1] = code;
//           std::cout << n << "-" << changeCount / 2 << " ";    // Muestra el nº de mensajes del mismo tipo y su longitud
        }
    }
    if (code == 0)
        return false;
    else
        return true;
}

