#ifndef __SOCKETSERVER__HPP
#define __SOCKETSERVER__HPP

#include <list>
#include <thread>
#include <string>

#include "EstacionMeteo.hpp"
#include "Anotador.hpp"

typedef std::thread * pthread;

class SocketServer
{
    public:
        SocketServer(std::string port, EstacionMeteo & estacion, Anotador & log);
        ~SocketServer();
        bool arranca();         //
        void termina();
    private:
        bool terminar;
        std::string port;
        pthread pt;                             // Thread escucha
        std::list<pthread> pt_list;             // Threads atiende_cliente's
        EstacionMeteo estacion;
        Anotador log;
        void atiende_cliente(int sd_client);    // Gestiona dialogo con un cliente. Esto en un thread
        void escucha();                         // Crea un socket de escucha y con select acepta clientes. Esto en un thread
};

#endif // __SOCKETSERVER__HPP
