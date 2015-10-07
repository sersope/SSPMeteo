#include "SocketServer.hpp"

SocketServer::SocketServer(std::string port)
{
    this->port = port;    terminar = false;
}

bool SocketServer::arranca()
{
    // TODO (sergio#1#07/10/15): Impedir un nuevo arranque
    pt = new std::thread(&SocketServer::escucha, this);
    return true;
}

void SocketServer::escucha()
{
    // Preparar/obtener la direccion del host y tipo de socket
    addrinfo host_info;                         // The struct that getaddrinfo() fills up with data.
    addrinfo *host_info_list;                   // Pointer to the to the linked list of host_info's.
    memset(&host_info, 0, sizeof host_info);
    host_info.ai_family = AF_UNSPEC;            // IP version not specified. Can be both.
    host_info.ai_socktype = SOCK_STREAM;        // Use SOCK_STREAM for TCP or SOCK_DGRAM for UDP.
    host_info.ai_flags = AI_PASSIVE;            // IP Wildcard
    int status = getaddrinfo(NULL, port.c_str(), &host_info, &host_info_list);
    if (status != 0)
        std::cout << "GETADDRINFO ERROR" << gai_strerror(status) ;

    // Crear el socket
    int sd_server = socket(host_info_list->ai_family, host_info_list->ai_socktype,  host_info_list->ai_protocol);
    if (sd_server == -1)
        std::cout << "SOCKET ERROR" ;

    // Unir el socket al host
    int yes = 1;
    status = setsockopt(sd_server, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)); // make sure the port is not in use by a previous execution of our code. (see man page for more information)
    status = bind(sd_server, host_info_list->ai_addr, host_info_list->ai_addrlen);
    if (status == -1)
        std::cout << "BIND ERROR" << std::endl ;

    freeaddrinfo(host_info_list);

    // Ponerse a la escucha
    std::cout << "A la escucha ..."  << std::endl;
    status =  listen(sd_server, 5);
    if (status == -1)
        std::cout << "LISTEN ERROR" << std::endl ;

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
            std::cout <<"SELECT ERROR";
            continue;
        }
        // Nueva conexion cliente. Se accepta y se crea el thread para atender las peticiones del cliente.
        if (FD_ISSET(sd_server, &readfds))
        {
            sockaddr_storage client_addr;
            socklen_t client_addr_s = sizeof(client_addr);
            int sd_client = accept(sd_server, (sockaddr *)&client_addr, &client_addr_s);
            pt_list.push_back(new std::thread(&SocketServer::atiende_cliente, this, sd_client));
            std::cout << "Nueva conexión aceptada (fd "  <<  sd_client << ")" << std::endl;
        }
    }
    std::cout << "Parando servidor ..." << std::endl;
    close(sd_server);
}
// TODO (sergio#1#06/10/15): Completar protocolo con cliente
void SocketServer::atiende_cliente(int sd_client)
{
    bool fin = false;
    char recv_buff[100];
    ssize_t bytes_recieved, bytes_sent;
    std::string msg_in, msg_out;

    while(!fin)
    {
        // Recibir
        bytes_recieved = recv(sd_client, recv_buff,100, 0);
        if (bytes_recieved == 0)
        {
            fin = true;
            continue;
        }
        if (bytes_recieved == -1)
        {
            std::cout << "RECIEVE ERROR" << std::endl ;
            fin = true;
            continue;
        }
        msg_in.assign(recv_buff,bytes_recieved);
        std::cout << " Recibido (fd " <<  sd_client << "):" << bytes_recieved << " " << msg_in;
        // Los caracteres de escape son para telnet
        if(msg_in == "getcurrent\r\n")
        {
            msg_out = "Has solicitado datos actuales\n";
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
            std::cout << "SEND ERROR" << std::endl ;
            fin = true;
            continue;
        }
    }
    close(sd_client);
    std::cout << "Conexión cerrada (fd "  <<  sd_client << ")" << std::endl;
}
void SocketServer::termina()
{
    terminar = true;
    pt->join();
}

SocketServer::~SocketServer()
{
    delete pt;
}
