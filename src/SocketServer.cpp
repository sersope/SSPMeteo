#include "SocketServer.hpp"

#include <unistd.h>     // Necesario para close()
#include <cstring>      // Needed for memset
#include <sys/socket.h> // Needed for the socket functions
#include <netdb.h>      // Needed for the socket functions
#include <sys/select.h> // Needed for the socket functions
#include <chrono>

SocketServer::SocketServer(std::string port, EstacionMeteo & estacion, Anotador & log)
{
    this->port = port;
    this->estacion = estacion;
    this->log = log;
    terminar = false;
    pt = 0;
}

bool SocketServer::arranca()
{
    if(!pt)
    {
        pt = new std::thread(&SocketServer::escucha, this);
        return true;
    }
    else
        return false;
}

void SocketServer::escucha()
{
    log.anota("Arrancando servidor ...");
    // Preparar/obtener la direccion del host y tipo de socket
    addrinfo host_info;                         // The struct that getaddrinfo() fills up with data.
    addrinfo *host_info_list;                   // Pointer to the to the linked list of host_info's.
    memset(&host_info, 0, sizeof host_info);
    host_info.ai_family = AF_UNSPEC;            // IP version not specified. Can be both.
    host_info.ai_socktype = SOCK_STREAM;        // Use SOCK_STREAM for TCP or SOCK_DGRAM for UDP.
    host_info.ai_flags = AI_PASSIVE;            // IP Wildcard
    int status = getaddrinfo(NULL, port.c_str(), &host_info, &host_info_list);
    if (status != 0)
        log.anota("ERROR AL HACER GETADDRINFO. SocketServer::escucha()") /*<< gai_strerror(status)*/ ;

    // Crear el socket
    int sd_server = socket(host_info_list->ai_family, host_info_list->ai_socktype,  host_info_list->ai_protocol);
    if (sd_server == -1)
        log.anota("ERROR AL CREAR EL SOCKET. SocketServer::escucha()");

    // Unir el socket al host
    int yes = 1;
    status = setsockopt(sd_server, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)); // make sure the port is not in use by a previous execution of our code. (see man page for more information)
    status = bind(sd_server, host_info_list->ai_addr, host_info_list->ai_addrlen);
    if (status == -1)
        log.anota("ERROR AL HACER BIND. SocketServer::escucha()");

    freeaddrinfo(host_info_list);

    // Ponerse a la escucha
    status =  listen(sd_server, 5);
    if (status == -1)
        log.anota("ERROR AL HACER LISTEN. SocketServer::escucha()");

    log.anota("Servidor a la escucha.");
    // Bucle de muestreo de nuevas conexiones
    fd_set readfds;
    timeval timeout;
    while(!terminar)
    {
        //clear the socket set, add socket server to read operations and wait for an activity with timeout.
        FD_ZERO(&readfds);
        FD_SET(sd_server, &readfds);
        timeout.tv_sec = 0;
        timeout.tv_usec = 100000; //microseconds
        int activity = select(sd_server + 1, &readfds, NULL, NULL, &timeout);
        if(activity < 0)
        {
            log.anota("ERROR EN SELECT. SocketServer::escucha()");
            continue;
        }
        // Nueva conexion cliente. Se accepta y se crea el thread para atender las peticiones del cliente.
        if (FD_ISSET(sd_server, &readfds))
        {
            sockaddr_storage client_addr;
            socklen_t client_addr_s = sizeof(client_addr);
            int sd_client = accept(sd_server, (sockaddr *)&client_addr, &client_addr_s);
            pt_list.push_back(new std::thread(&SocketServer::atiende_cliente, this, sd_client));
            log.anota("Nueva conexión cliente aceptada.");
        }
    }
    close(sd_server);
    log.anota("Conexión servidor cerrada.");
}
// TODO (sergio#1#06/10/15): Completar protocolo con cliente
void SocketServer::atiende_cliente(int sd_client)
{
    bool fin = false;
    char recv_buff[100];
    ssize_t bytes_recieved, bytes_sent;
    std::string msg_in, msg_out;

    while(!fin && !terminar)
    {
        // Recibir sin esperar
        bytes_recieved = recv(sd_client, recv_buff,100, MSG_DONTWAIT);
        if (bytes_recieved == 0)
        {
            fin = true;
            continue;
        }
        if (bytes_recieved == -1) //No se ha recibido nada o error
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10)); //Espera y vueleve a recibir
            continue;
        }
        //Algo se ha recibido
        msg_in.assign(recv_buff,bytes_recieved);
        // Los caracteres de escape son para poder conectarse por telnet
        if(msg_in == "getcurrent\r\n")
        {
            msg_out = estacion.getcurrent();
        }
        else if(msg_in == "getdaily\r\n")
        {
            msg_out = "Has solicitado datos diarios\n";
        }
        else if(msg_in == "quit\r\n")
        {
            msg_out = "Adiós\n";
            fin = true;
        }
        else
            msg_out = "Pardon?\n";
        // Enviar
        bytes_sent = send(sd_client, msg_out.c_str(), msg_out.length(), 0);
        if (bytes_sent == -1)
        {
            log.anota("ERROR AL HACER SEND. SocketServer::atiende_cliente()");
            fin = true;
            continue;
        }
    }
    close(sd_client);
    log.anota("Conexión cliente cerrada.");
}
void SocketServer::termina()
{
    // Cerrando threads servidor y threads de clientes
    log.anota("Cerrando servidor...");
    terminar = true;
    pt->join();
    for( auto it = pt_list.begin(); it != pt_list.end(); it++)
    {
        if((*it)->joinable())
        {
            log.anota("Cerrando cliente...");
            (*it)->join();
        }
    }
}

SocketServer::~SocketServer()
{
    delete pt;
}
