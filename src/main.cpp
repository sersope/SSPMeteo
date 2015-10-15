#include <iostream>
#include <string>
//#include <wiringPi.h> // Para el delay en main()

#include "EstacionMeteo.hpp"
#include "SocketServer.hpp"

int main(int argc, char *argv[])
{

    EstacionMeteo estacion;
    std::cout << "\nEstación meteo arrancando...\n";
    if (!estacion.arranca())
    {
        std::cout << "Estación meteo no pudo arrancar.\n" << std::endl;
        return 0;
    }

    SocketServer sserver("5556", estacion);
    sserver.arranca();

    std::cout << std::endl << "Pulse q(Q) para terminar..." << std::endl;
    bool salir = false;
    std::string tecla;
    while(!salir)
    {
        std::cin >> tecla;
        salir = (tecla=="q" | tecla=="Q");
//        std::cout << " Temperatura: " << estacion.getT() << " Humedad: " << estacion.getH() << " Lluvia: " << estacion.getR() << std::endl;
//        delay(3000);
    }

    // Se debe parar el servidor
    sserver.termina();

    return 0;
}
