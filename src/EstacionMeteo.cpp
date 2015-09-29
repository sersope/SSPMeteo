/*
    Codificación de datos:
    -   nº del mensaje [1..255] 8 bits
    -   temperatura [-30,0ºC, 70,0ºC]    -> codigo temperatura= int(temperatura * 10) + 300 [0,1000]  Error de lectura:
    -   humedad     [0.0%, 100.0%]       -> codigo humedad= int(humedad * 10)               [0,1000]  Error de lectura:
    -   tics lluvia [0, 65535] 16 bits.

    mensaje1: unsigned long. 32 bits
        00000000 00000001 tttttttttttttttt -> temperatura.
    mensaje2: unsigned long. 32 bits
        00000000 00000010 hhhhhhhhhhhhhhhh -> humedad.
    mensaje3: unsigned long. 32 bits
        00000000 00000011 rrrrrrrrrrrrrrrr -> lluvia.
*/
#include "EstacionMeteo.hpp"
#include "ReceptorRF433.hpp"

bool EstacionMeteo::arranca()
{
    return ReceptorRF433::arranca();
}

float EstacionMeteo::getT()
{
    temp = ((ReceptorRF433::mensaje_tipo[0] & 0xFFFF) - 300.0) / 10.0;
    return temp;
}

float EstacionMeteo::getH()
{
    humi = (ReceptorRF433::mensaje_tipo[1] & 0xFFFF) / 10.0;
    return humi;
}

unsigned EstacionMeteo::getR()
{
    rain = ReceptorRF433::mensaje_tipo[2] & 0xFFFF;
    return rain;
}
