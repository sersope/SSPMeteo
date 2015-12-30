 /*
    Codificación de datos:
    -   nº del mensaje [1..255] 8 bits
    // TODO (sergio#1#): Determinar códigos de error para los valores erróneos
    -   temperatura [-30,0ºC, 70,0ºC] float    -> codigo temperatura= word(temperatura * 10) + 300 [0,1000]  Error de lectura:
    -   humedad     [0.0%, 100.0%]    float    -> codigo humedad= word(humedad * 10)               [0,1000]. Error de lectura:
    -   tics lluvia [0, 65535] 16 bits. word
    -   velocidad viento [0.0, 160.0 km/h]     float    -> codigo vel. vent. = word(vel_vent * 10)
    -   velocidad racha  [0.0, 160.0 km/h]     float    -> codigo vel. racha = word(vel_racha *10)
    -   direccion viento [0, 359]     word
    mensaje1: unsigned long. 32 bits
        0000000000000001tttttttttttttttt -> temperatura.
    mensaje2: unsigned long. 32 bits
        0000000000000010hhhhhhhhhhhhhhhh -> humedad.
    mensaje3: unsigned long. 32 bits
        0000000000000011rrrrrrrrrrrrrrrr -> lluvia.
    mensaje4: unsigned long. 32 bits
        0000000000000100rrrrrrrrrrrrrrrr -> velocidad del viento.
    mensaje5: unsigned long. 32 bits
        0000000000000101rrrrrrrrrrrrrrrr -> velocidad de racha
    mensaje6: unsigned long. 32 bits
        0000000000000110rrrrrrrrrrrrrrrr -> direccion del viento
*/

#include "EstacionMeteo.hpp"
#include "ReceptorRF433.hpp"

#include <sstream>
#include <ctime>
#include <thread>
#include <Anotador.hpp>

EstacionMeteo::EstacionMeteo()
{
    pth = 0;
}

EstacionMeteo::~EstacionMeteo()
{
    //delete pth;
}

bool EstacionMeteo::arranca()
{
    terminar = false;
    pth = new std::thread(&EstacionMeteo::procesa, this);

    temp = -70.0;
    humi = 0.0;
    rain = 0;
    vel_vent = 0.0;
    vel_racha = 0.0;
    dir_vent = 0;
    return ReceptorRF433::arranca();
    //return true;
}

void EstacionMeteo::termina()
{
    if(pth != 0)
    {
        terminar = true;
        pth->join();
    }
}

void EstacionMeteo::procesa()
{
    Anotador datos_log("datos.dat");
    double periodo_lectura = 60.0;          // segundos
    double periodo_salvadatos = 1 * 60.0;  // segundos
    time_t timer_lectura,timer_salvadatos,ahora;
    time(&ahora);
    timer_lectura = ahora;
    timer_salvadatos = ahora;
    int hoy = localtime(&ahora)->tm_mday;
    int mes = localtime(&ahora)->tm_mon + 1;
    int anyo = localtime(&ahora)->tm_year + 1900;
    int ayer = hoy;

    //Genera el nombre del fichero de datos diarios
    std::stringstream nomfile;
    nomfile.str("");
    nomfile << anyo << "-" << mes << "-" << hoy << ".dat";
    datos_log.setName(nomfile.str());

    while(!terminar)
    {
        time(&ahora);
        hoy = localtime(&ahora)->tm_mday;

        // Por cambio de dia
        if(hoy != ayer)
        {
            // Cambio de dia.
            // Obten nuevo nombre para el fichero de datos diarios
            mes = localtime(&ahora)->tm_mon + 1;
            anyo = localtime(&ahora)->tm_year + 1900;
            nomfile.str("");
            nomfile << anyo << "-" << mes << "-" << hoy << ".dat";
            datos_log.setName(nomfile.str());

            ayer = hoy;
        }
        if(difftime(ahora,timer_lectura) >= periodo_lectura)
        {
            // TODO (sergio#1#30/12/15): Añadir envio a weather underground
            timer_lectura = ahora;
        }
        if(difftime(ahora,timer_salvadatos) >= periodo_salvadatos)
        {
            // Salva datos actuales
            datos_log.anota(getcurrent());

            timer_salvadatos = ahora;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

float EstacionMeteo::getT(char unit)
{
    if( esMensajeBueno(0) )
    {
        float aux;
        aux = ((ReceptorRF433::mensaje_tipo[0][0] & 0xFFFF) - 300.0) / 10.0;
        if( aux >= -30.0 && aux <= 70.0 )
            temp = aux;
    }
    return temp;
}

float EstacionMeteo::getH()
{
    if( esMensajeBueno(1) )
    {
        float aux;
        aux = (ReceptorRF433::mensaje_tipo[1][0] & 0xFFFF) / 10.0;
        if( aux >= 0.0 && aux <= 100.0 )
            humi = aux;
    }
    return humi;
}

unsigned EstacionMeteo::getR(char unit)
{
    if( esMensajeBueno(2) )
    {
        int aux;
        aux = ReceptorRF433::mensaje_tipo[2][0] & 0xFFFF;
        if( aux >= 0 && aux <= 65535 )
            rain = aux;
    }
    return rain;
}

float EstacionMeteo::getVV(char unit)
{
    if( esMensajeBueno(3) )
    {
        float aux;
        aux = (ReceptorRF433::mensaje_tipo[3][0] & 0xFFFF) / 10.0;
        if( aux >= 0.0 && aux <= 160.0 )
            vel_vent = aux;
    }
    return vel_vent;
}

float EstacionMeteo::getVR(char unit)
{
    if( esMensajeBueno(4) )
    {
        float aux;
        aux = (ReceptorRF433::mensaje_tipo[4][0] & 0xFFFF) / 10.0;
        if( aux >= 0.0 && aux <= 160.0 )
            vel_racha = aux;
    }
    return vel_racha;
}

unsigned EstacionMeteo::getDV()
{
    if( esMensajeBueno(5) )
    {
        int aux;
        aux = ReceptorRF433::mensaje_tipo[5][0] & 0xFFFF;
        if( aux >= 0 && aux < 360 )
            dir_vent = aux;
    }
    return dir_vent;
}

std::string EstacionMeteo::getcurrent()
{
    std::stringstream ss;
    ss << getT() << "," << getH() << "," << getR() << "," << getVV() << "," << getVR() << "," << getDV() << "," << ReceptorRF433::mensajes_recibidos;
    for( int i = 0; i < 6; i++)
        ss << "," << ReceptorRF433::mensaje_indice[i];
    return ss.str();
}

// Filtrado de mensajes recibidos.
// Por lo menos tres mensajes y todos iguales
bool EstacionMeteo::esMensajeBueno(int nmen)
{
    int n = ReceptorRF433::mensaje_indice[nmen];

    if( n < 3)
        return false;

    unsigned valor = ReceptorRF433::mensaje_tipo[nmen][0];
    for(int i = 0; i < n; i++)
        if( ReceptorRF433::mensaje_tipo[nmen][i] != valor)
            return false;
    return true;
}
