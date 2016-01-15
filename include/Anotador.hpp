#ifndef __ANOTADOR__HPP__
#define __ANOTADOR__HPP__

#include <string>

/** \brief Clase para anotar información en un fichero.
 *
 * Se usa un fichero para anotar información en forma de mensajes.
 * Si el fichero no existe se crea nuevo.
 * Si el fichero existe se añaden los nuevos mensajes al final.
 * Cada mensaje ocupa un línea del fichero.
 * A cada mensaje anotado se le antepone la fecha y hora en la que se produce.
 */
class Anotador
{
    public:
        /// \brief Constructor por defecto.
        Anotador();
        /// \brief Constructor con el nombre del ficehro.
        Anotador(const std::string & fname);
        /// \brief Cambia el nombre del fichero.
        void setName(const std::string & fname);
        /// \brief Añade un nuevo mensaje en el fichero.
        void anota(const std::string & texto,char t='l');
        /// Obtiene la fecha y hora formateadas.
        std::string str_ahora(char t='l');
    private:
        std::string fname;
};

#endif // __ANOTADOR__HPP__
