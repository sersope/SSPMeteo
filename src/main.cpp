#include <fstream>
#include <iostream>

#include "EstacionMeteo.hpp"

int main(int argc, char *argv[])
{
    EstacionMeteo estacion;

    if (!estacion.arranca())
    {
        std::cout << "No se pudo conectar con la estación.." << std::endl;
        return 0;
    }

    std::ofstream outf("Mensajes.dat", std::ios::app);
    while(true)
    {
        std::cout << " Temperatura: " << estacion.getT() << " Humedad: " << estacion.getH() << " Lluvia: " << estacion.getR() << std::endl;
        outf << " Temperatura: " << estacion.getT() << " Humedad: " << estacion.getH() << " Lluvia: " << estacion.getR() << std::endl;
        delay(3000);
    }
    return 0;
}
