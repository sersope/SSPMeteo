#ifndef __SOCKETSERVER__HPP
#define __SOCKETSERVER__HPP

#include <unistd.h>     // Necesario para close()
#include <iostream>
#include <cstring>      // Needed for memset
#include <sys/socket.h> // Needed for the socket functions
#include <netdb.h>      // Needed for the socket functions
#include <sys/select.h>
#include <list>
#include <thread>
#include <string>

#include "EstacionMeteo.hpp"

typedef std::thread * pthread;

// TODO (sergio#1#07/10/15): Considerar log file para esta clase
// TODO (sergio#1#09/10/15): Añadir miembro private EstacionMeteo
class SocketServer
{
    public:
        SocketServer(std::string port, EstacionMeteo & estacion);
        ~SocketServer();
        bool arranca();         //
        void termina();
    private:
        bool terminar;
        std::string port;
        pthread pt;                             // Thread escucha
        std::list<pthread> pt_list;             // Threads atiende_cliente's
        EstacionMeteo estacion;

        void atiende_cliente(int sd_client);    // Gestiona dialogo con un cliente. Esto en un thread
        void escucha();                         // Crea un socket de escucha y con select acepta clientes. Esto en un thread
};

#endif // __SOCKETSERVER__HPP
