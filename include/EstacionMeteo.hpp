#ifndef __ESTACION__METEO__HPP
#define __ESTACION__METEO__HPP

#include <string>

/** \brief Este namespace encapsula el funcionamiento de la estación meteorológica.
 *
 */
namespace EstacionMeteo
{
        /// Lanza el proceso continuo de ejecución.
        bool arranca();
        /// Este es el proceso continuo de ejecución.
        void procesa();
        /// Finaliza el proceso continuo de ejecución.
        void termina();
        /// Devuelve un string con los datos actuales de la estación.
        std::string getcurrent();
}
#endif // __ESTACION__METEO__HPP
