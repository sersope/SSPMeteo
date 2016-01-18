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
        /// Devuelve un string con los datos actuales de la estación.
        std::string getcurrent();
        /// Finaliza el proceso continuo de ejecución.
        void termina();
}
#endif // __ESTACION__METEO__HPP
