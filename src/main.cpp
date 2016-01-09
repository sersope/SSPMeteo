#include <iostream>
#include <string>

#include "EstacionMeteo.hpp"
#include "SocketServer.hpp"
#include "Anotador.hpp"

int main(int argc, char *argv[])
{

    Anotador thelog("sspmeteo.log");
    thelog.anota("+++ NUEVO ARRANQUE +++");

    if (!EstacionMeteo::arranca())
    {
        //std::cout << "Estación meteo no pudo arrancar.\n" << std::endl;
        thelog.anota("Estación meteo no pudo arrancar.");
        return 0;
    }

    SocketServer sserver("5556");
    sserver.arranca();

    //std::cout << std::endl << "Pulse q(Q)+Intro  para terminar..." << std::endl;
    //bool salir = false;
    //std::string tecla;

    while(!sserver.terminar)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        //std::cin >> tecla;
        //salir = (tecla=="q" || tecla=="Q");
    }

    // Se debe parar el servidor
    EstacionMeteo::termina();
    sserver.termina();

    thelog.anota("--- FIN  ---");

    return 0;
}
