#include "Anotador.hpp"

#include <fstream>
#include <locale>
#include <ctime>

/** Crea un objeto Anotador con nombre de fichero "anotador.log".
 */
Anotador::Anotador()
{
    fname= "anotador.log";
}

/** Crea un objeto con un nombre de fichero dado.
 * \param fname const std::string& Nombre del fichero
 *
 */
Anotador::Anotador(const std::string & fname)
{
    this->fname = fname;
}

/** Pon el nombre al fichero.
 * \param fname const std::string& Nombre del fichero
 * \return void
 */
void Anotador::setName(const std::string & fname)
{
    this->fname = fname;
}

/** Añade una línea al fichero con la fecha y hora actuales.
 * El formato es fecha,hora,texto.\see str_ahora()
 * \param texto const std::string& Texto del mensaje.
 * \param t char Indica si la hora es local ('l') o utc ('u'). Por defecto es local.
 * \return void
 *
 */
void Anotador::anota(const std::string & texto, char t)
{
    std::ofstream file(fname, std::ofstream::app);

    file << str_ahora(t) << "," << texto << std::endl;
    file.flush();
    file.close();
}

/** Devuelve un string con la hora y fecha actuales.
 * El formato es fecha,hora.
 * La fecha está en formato YYYY-MM-DD.
 * La hora está en formato HH:MM:SS.
 * \param t char Si es 'l' en local, si es 'u' en utc.
 * \return std::string "Fecha,hora" formateadas.
 *
 */
std::string Anotador::str_ahora(char t)
{

    time_t rawtime;
    struct tm * timeinfo;
    char buffer[25];

    time(&rawtime);
    if(t == 'u')
        timeinfo = gmtime(&rawtime);
    else
        timeinfo = localtime(&rawtime);
    strftime(buffer,25, "%F,%T", timeinfo);
    return std::string(buffer);
}
