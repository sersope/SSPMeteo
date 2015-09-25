/*

*/
#ifndef __RECEPTOR__RF__433_HPP
#define __RECEPTOR__RF__433_HPP

#include <wiringPi.h>
#include <stdint.h>

// Number of maximum High/Low changes per packet.
// We can handle up to (unsigned long) => 32 bit * 2 H/L changes per bit + 2 for sync
//const int PACKET_MAX_CHANGES = 67;

class ReceptorRF433
{
  public:
    ReceptorRF433();
    void enableReceive(int interrupt);
    void disableReceive();
    bool available();
	void resetAvailable();
    unsigned long getReceivedValue();

  private:
    static void handleInterrupt();
	static bool receiveProtocol1(unsigned int changeCount);
    int nReceiverInterrupt;

	static const int nReceiveTolerance = 60;    // Valor constante
    static const int PACKET_MAX_CHANGES = 67;   // Valor constante
    static unsigned long nReceivedValue;        // Aqui está el mensaje recibido o es 0
    static unsigned int nReceivedBitlength;     // Valor solo informativo
	static unsigned int nReceivedDelay;         // Valor solo informativo
	static unsigned int nReceivedProtocol;      // Valor solo informativo
    static unsigned int timings[ReceptorRF433::PACKET_MAX_CHANGES];

};
#endif //__RECEPTOR__RF__433_HPP
