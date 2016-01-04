#include "Anotador.hpp"

#include <fstream>
#include <locale>
#include <ctime>

Anotador::Anotador()
{
    fname= "anotador.log";
}
Anotador::Anotador(const std::string & fname)
{
    this->fname = fname;
}

void Anotador::setName(const std::string & fname)
{
    this->fname = fname;
}

void Anotador::anota(const std::string & texto, char t)
{
    std::ofstream file(fname, std::ofstream::app);

    file << str_ahora(t) << "," << texto << std::endl;
    file.flush();
    file.close();
}

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
