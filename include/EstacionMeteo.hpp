#ifndef __ESTACION__METEO__HPP
#define __ESTACION__METEO__HPP

#include <string>

/** \brief Este namespace encapsula el funcionamiento de la estación meteorológica.
 *
 */
namespace EstacionMeteo
{
        /// Lanza el servidor de sockets y el receptor RF433.
        bool arranca();
        /// Este es el bucle continuo de ejecución.
        void procesa();
        /// Cierra el servidor de sockets y posibles clientes.
        void termina();
        /// Devuelve un string con los datos actuales de la estación.
        std::string getcurrent();
}
#endif // __ESTACION__METEO__HPP
