#include <fstream>
#include <iostream>
#include <wiringPi.h> // Para el delay en main()

#include "EstacionMeteo.hpp"
#include "SocketServer.hpp"

//#define NO_RPI  // Comentar para Raspberry Pi

int main(int argc, char *argv[])
{
    SocketServer sserver("5556");
    sserver.arranca();

#ifndef NO_RPI
    EstacionMeteo estacion;
    std::cout << "\nEstación meteo arrancando...\n";
    if (!estacion.arranca())
    {
        std::cout << "No se pudo conectar con la estación..\n" << std::endl;
        return 0;
    }
#endif // NO_RPI

//    std::ofstream outf("Mensajes.dat", std::ios::app);
    while(true)
    {
//        std::cout << " Temperatura: " << estacion.getT() << " Humedad: " << estacion.getH() << " Lluvia: " << estacion.getR() << std::endl;
//        outf << " Temperatura: " << estacion.getT() << " Humedad: " << estacion.getH() << " Lluvia: " << estacion.getR() << std::endl;
        delay(3000);
    }

    // Se debe parar el servidor

    return 0;
}
