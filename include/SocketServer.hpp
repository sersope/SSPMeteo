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

typedef std::thread * pthread;

class SocketServer
{
    public:
        SocketServer(std::string port);
        ~SocketServer();
        bool arranca();         //
        void termina();
    private:
        bool terminar;
        std::string port;
        pthread pt;                             // Thread escucha
        std::list<pthread> pt_list;             // Threads atiende_cliente's

        void atiende_cliente(int sd_client);    // Gestiona dialogo con un cliente. Esto en un thread
        void escucha();                         // Crea un socket de escucha y con select acepta clientes. Esto en un thread
};

#endif // __SOCKETSERVER__HPP
