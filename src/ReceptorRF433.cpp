/*
*/

#include "ReceptorRF433.hpp"

unsigned long ReceptorRF433::nReceivedValue = 0;
unsigned int ReceptorRF433::nReceivedBitlength = 0;
unsigned int ReceptorRF433::nReceivedDelay = 0;
unsigned int ReceptorRF433::nReceivedProtocol = 0;
unsigned int ReceptorRF433::timings[ReceptorRF433::PACKET_MAX_CHANGES];

ReceptorRF433::ReceptorRF433()
{
    this->nReceiverInterrupt = -1;
    ReceptorRF433::nReceivedValue = 0;
}

/**
 * Enable receiving data
 */
void ReceptorRF433::enableReceive(int interrupt)
{
    this->nReceiverInterrupt = interrupt;
    if (this->nReceiverInterrupt != -1)
    {
        ReceptorRF433::nReceivedValue = 0;
        ReceptorRF433::nReceivedBitlength = 0;
        wiringPiISR(this->nReceiverInterrupt, INT_EDGE_BOTH, &handleInterrupt);
    }
}

/**
 * Disable receiving data
 */
void ReceptorRF433::disableReceive()
{
    this->nReceiverInterrupt = -1;
}

bool ReceptorRF433::available()
{
    return ReceptorRF433::nReceivedValue != 0;
}

void ReceptorRF433::resetAvailable()
{
    ReceptorRF433::nReceivedValue = 0;
}

unsigned long ReceptorRF433::getReceivedValue()
{
    return ReceptorRF433::nReceivedValue;
}

bool ReceptorRF433::receiveProtocol1(unsigned int changeCount)
{
    unsigned long code = 0;
    unsigned long delay = ReceptorRF433::timings[0] / 31;
    unsigned long delayTolerance = delay * ReceptorRF433::nReceiveTolerance * 0.01;

    for (unsigned i = 1; i<changeCount ; i=i+2)
    {

        if (ReceptorRF433::timings[i] > delay-delayTolerance && ReceptorRF433::timings[i] < delay+delayTolerance && ReceptorRF433::timings[i+1] > delay*3-delayTolerance && ReceptorRF433::timings[i+1] < delay*3+delayTolerance)
        {
            code = code << 1;
        }
        else if (ReceptorRF433::timings[i] > delay*3-delayTolerance && ReceptorRF433::timings[i] < delay*3+delayTolerance && ReceptorRF433::timings[i+1] > delay-delayTolerance && ReceptorRF433::timings[i+1] < delay+delayTolerance)
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
    if (changeCount > 6)      // ignore < 4bit values as there are no devices sending 4bit values => noise
    {
        ReceptorRF433::nReceivedValue = code;
        ReceptorRF433::nReceivedBitlength = changeCount / 2;
        ReceptorRF433::nReceivedDelay = delay;
        ReceptorRF433::nReceivedProtocol = 1;
        // TODO (sergio#1#25/09/15): Crear buffer de mensajes y asignar el mensaje a este buffer.
    }

    if (code == 0)
    {
        return false;
    }
    else// if (code != 0)
    {
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

    if (duration > 5000 && duration > ReceptorRF433::timings[0] - 200 && duration < ReceptorRF433::timings[0] + 200)
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

    if (changeCount >= ReceptorRF433::PACKET_MAX_CHANGES)
    {
        changeCount = 0;
        repeatCount = 0;
    }
    ReceptorRF433::timings[changeCount++] = duration;
    lastTime = time;
}
