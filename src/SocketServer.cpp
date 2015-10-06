#include "SocketServer.hpp"

SocketServer::SocketServer(std::string port)
{
    this->port = port;
    terminar = false;
}

bool SocketServer::arranca()
{
    pt = new std::thread(&SocketServer::escucha, this);
    return true;
}

void SocketServer::escucha()
{
    std::cout << "Setting up the structs..."  << std::endl;
    addrinfo host_info;                     // The struct that getaddrinfo() fills up with data.
    addrinfo *host_info_list;               // Pointer to the to the linked list of host_info's.
    memset(&host_info, 0, sizeof host_info);
    host_info.ai_family = AF_UNSPEC;        // IP version not specified. Can be both.
    host_info.ai_socktype = SOCK_STREAM;    // Use SOCK_STREAM for TCP or SOCK_DGRAM for UDP.
    host_info.ai_flags = AI_PASSIVE;        // IP Wildcard

    int status = getaddrinfo(NULL, port.c_str(), &host_info, &host_info_list);
    if (status != 0)  std::cout << "getaddrinfo error" << gai_strerror(status) ;

    std::cout << "Creating a socket..."  << std::endl;
    int sd_server = socket(host_info_list->ai_family, host_info_list->ai_socktype,  host_info_list->ai_protocol);
    if (sd_server == -1)  std::cout << "socket error " ;

    std::cout << "Binding socket..."  << std::endl;
    // we use to make the setsockopt() function to make sure the port is not in use
    // by a previous execution of our code. (see man page for more information)
    int yes = 1;
    status = setsockopt(sd_server, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    status = bind(sd_server, host_info_list->ai_addr, host_info_list->ai_addrlen);
    if (status == -1)  std::cout << "bind error" << std::endl ;

    std::cout << "Listen()ing for connections..."  << std::endl;
    status =  listen(sd_server, 5);
    if (status == -1)  std::cout << "listen error" << std::endl ;
    //select
    //set of socket descriptors
    fd_set readfds;
    while(!terminar)
    {
        //clear the socket set, add socketEscucha to read set and wait for an activity on the socket with timeout.
        FD_ZERO(&readfds);
        FD_SET(sd_server, &readfds);
        timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 100000; //microseconds
        int activity = select( sd_server + 1 , &readfds , NULL , NULL , &timeout);
        if(activity < 0)
        {
            std::cout <<"select error";
        }

        //On the server socket, an incoming connection
        if (FD_ISSET(sd_server, &readfds))
        {
            std::cout <<"NUEVO CLIENTE !!!!";
            sockaddr_storage client_addr;
            socklen_t client_addr_s = sizeof(client_addr);
            int sd_client = accept(sd_server, (sockaddr *)&client_addr, &client_addr_s);
            pt_list.push_back(new std::thread(&SocketServer::atiende_cliente, this, sd_client));
            std::cout << "Connection accepted. Using new socketfd : "  <<  sd_client << std::endl;
        }
    }
    std::cout << "Stopping server..." << std::endl;
    freeaddrinfo(host_info_list);
    close(sd_server);
}
// TODO (sergio#1#06/10/15): Añadir protocolo con cliente
void SocketServer::atiende_cliente(int sd_client)
{
    bool fin = false;
    char incomming_data_buffer[1000];
    ssize_t bytes_recieved;
    ssize_t bytes_sent;

    while(!fin)
    {
        std::cout << "Waiting to recieve data..."  << std::endl;
        bytes_recieved = recv(sd_client, incomming_data_buffer,1000, 0);
        // If no data arrives, the program will just wait here until some data arrives.
        if (bytes_recieved == 0)
        {
            std::cout << "El cliente se cerró." << std::endl ;
            fin = true;
            continue;
        }
        if (bytes_recieved == -1)
        {
            std::cout << "recieve error!" << std::endl ;
            fin = true;
            continue;
        }
        std::cout << bytes_recieved << " bytes recieved :" << std::endl ;
        incomming_data_buffer[bytes_recieved] = '\0';
        std::cout << incomming_data_buffer << std::endl;


        std::cout << "send()ing back a message..."  << std::endl;
        std::string msg("thank you.\n");
        bytes_sent = send(sd_client, msg.c_str(), msg.length(), 0);
    }
    close(sd_client);
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
