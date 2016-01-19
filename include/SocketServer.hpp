#ifndef __SOCKETSERVER__HPP
#define __SOCKETSERVER__HPP

#include <list>
#include <thread>
#include <string>

typedef std::thread * pthread;

/** \brief Clase para la gestión de la comunicación con clientes
 *
 * Una vez creado el objeto se debe llamar al método arranca().
 * Para terminar la comunicación se llamará al método termina().
 *
 * Se lanza un thread que abre un sockect port a la escucha. Por cada solicitud
 * de un cliente se lanza un thread para la gestión de la comunicación con el cliente.
 * El cliente puede solicitar lo siguiente:
 * - Obtener los datos actuales de la estación ("getcurrent\r\n"). Se devuelve un string con los valores separados por comas.
 * - Parar el servidor ("stopserver\r\n"). Se devuelve "Adiós\n".
 * - Terminar la comunicación ("quit\r\n"). Se devuelve "Adiós\n".
 *
 * En caso de no entender la petición el server devuelve "Pardon?\n".
 */
class SocketServer
{
    public:
        ///
        /** \brief Constructor con el puerto socket para la escucha.
         *
         * \param port std::string. Puerto para la escucha.
         *
         */
        SocketServer(std::string port);
        ~SocketServer();
        /// Se lanza el trhead para la escucha.
        bool arranca();
        /// Variable para señalizar la finalización de las comunicaciones
        bool terminar;
        /// Finaliza toda la gestión de las comunicaciones
        void termina();
    private:
        std::string port;
        pthread pt;                             // Thread escucha
        std::list<pthread> pt_list;             // Threads atiende_cliente's
        void atiende_cliente(int sd_client);    // Gestiona dialogo con un cliente. Esto en un thread
        void escucha();                         // Crea un socket de escucha y con select acepta clientes. Esto en un thread
};

#endif // __SOCKETSERVER__HPP
