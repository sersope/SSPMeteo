/*
    Codificación de datos:
    -   nº del mensaje [1..255] 8 bits
    // TODO (sergio#1#): Determinar códigos de error para los valores erróneos
    -   temperatura [-30,0ºC, 70,0ºC]    -> codigo temperatura= int(temperatura * 10) + 300 [0,1000]  Error de lectura:
    -   humedad     [0.0%, 100.0%]       -> codigo humedad= int(humedad * 10)               [0,1000]  Error de lectura:
    -   tics lluvia [0, 65535] 16 bits.

    mensaje1: unsigned long. 32 bits
        0000000000000001tttttttttttttttt -> temperatura.
    mensaje2: unsigned long. 32 bits
        0000000000000010hhhhhhhhhhhhhhhh -> humedad.
    mensaje3: unsigned long. 32 bits
        0000000000000011rrrrrrrrrrrrrrrr -> lluvia.
*/
#include "EstacionMeteo.hpp"

EstacionMeteo::EstacionMeteo()
{
    mensaje = 0;
    mensaje_ant = 0;
    parar = false;
}

bool EstacionMeteo::arranca()
{
    if(wiringPiSetup() == -1)
        return false;
    else
    {
        receptor.enableReceive(RF_PIN);
        philo = new std::thread(&EstacionMeteo::procesa, this);
        return true;
    }
}

void EstacionMeteo::procesa()
{
    int n = 0;
    while(!parar)
    {
        mensaje = 0;
        if (receptor.available())
        {
            mensaje = receptor.getReceivedValue();
            receptor.resetAvailable();
            n = (mensaje & 0xFF0000) >> 16; //Nº del mensaje
            printf("%d",n); //DEBUG
            if (n >= 1 && n <= N_MENSAJES && mensaje != mensaje_ant)
            {
                switch (n)
                {
                case 1:
                    datos.temp = ((mensaje & 0xFFFF) - 300.0) / 10.0;
                    break;
                case 2:
                    datos.humi = (mensaje & 0xFFFF) / 10.0;
                    break;
                case 3:
                    datos.rain = mensaje & 0xFFFF;
                    break;
                }
                mensaje_ant = mensaje;
            }
        }
    }
}

bool EstacionMeteo::para()
{
    parar = true;
    philo->join();
    return true;
}

EstacionMeteo::~EstacionMeteo()
{
    delete philo;
    printf("destructor\n");
}
