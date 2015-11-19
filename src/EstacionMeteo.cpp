/*
    Codificación de datos:
    -   nº del mensaje [1..255] 8 bits
    -   temperatura [-30,0ºC, 70,0ºC]    -> codigo temperatura= int(temperatura * 10) + 300 [0,1000]  Error de lectura:
    -   humedad     [0.0%, 100.0%]       -> codigo humedad= int(humedad * 10)               [0,1000]  Error de lectura:
    -   tics lluvia [0, 65535] 16 bits.

    mensaje1: unsigned long. 32 bits
        00000000 00000001 tttttttttttttttt -> temperatura.
    mensaje2: unsigned long. 32 bits
        00000000 00000010 hhhhhhhhhhhhhhhh -> humedad.
    mensaje3: unsigned long. 32 bits
        00000000 00000011 rrrrrrrrrrrrrrrr -> lluvia.
*/
#include "EstacionMeteo.hpp"
#include "ReceptorRF433.hpp"

#include <sstream>
#include <cereal/archives/json.hpp>
#include <cereal/types/vector.hpp>
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
//////    return ReceptorRF433::arranca();
return true;
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
    Anotador log("EstacionMeteo.log");
    double periodo_lectura = 60.0;          // segundos
    double periodo_salvadatos = 15 * 60.0;  // segundos
    time_t timer_lectura,timer_salvadatos,ahora;
    time(&ahora);
    timer_lectura = ahora;
    timer_salvadatos = ahora;
    int hoy = localtime(&ahora)->tm_mday;
    int ayer = hoy;

    while(!terminar)
    {
        time(&ahora);
        hoy = localtime(&ahora)->tm_mday;
        if(difftime(ahora,timer_lectura) >= periodo_lectura)
        {
            // TODO: Leer datos actuales.
            // Actualizar datos diarios
            log.anota("Lectura de datos");
            timer_lectura = ahora;
        }
        if(difftime(ahora,timer_salvadatos) >= periodo_salvadatos)
        {
            // Salva datos actuales
            log.anota("Salvado de datos");
            timer_salvadatos = ahora;
        }
        if(hoy != ayer)
        {
            // Cambio de dia.
            // Salvar datos diarios
            // Resetear datos diarios
            log.anota("Cambio de dia");
            ayer = hoy;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }
}

float EstacionMeteo::getT(char unit)
{
    temp = ((ReceptorRF433::mensaje_tipo[0] & 0xFFFF) - 300.0) / 10.0;
    return temp;
}

float EstacionMeteo::getH()
{
    humi = (ReceptorRF433::mensaje_tipo[1] & 0xFFFF) / 10.0;
    return humi;
}

unsigned EstacionMeteo::getR(char unit)
{
    rain = ReceptorRF433::mensaje_tipo[2] & 0xFFFF;
    return rain;
}

float EstacionMeteo::getVV(char unit)
{
    vel_vent = (ReceptorRF433::mensaje_tipo[3] & 0xFFFF) / 10.0;
    return vel_vent;
}

float EstacionMeteo::getVR(char unit)
{
    vel_racha = (ReceptorRF433::mensaje_tipo[4] & 0xFFFF) / 10.0;
    return vel_racha;
}

unsigned EstacionMeteo::getDV()
{
    dir_vent = ReceptorRF433::mensaje_tipo[5] & 0xFFFF;
    return dir_vent;
}

std::string EstacionMeteo::getcurrent()
{
    std::stringstream ss;
    {
        cereal::JSONOutputArchive archive( ss );
        archive(cereal::make_nvp("temp",getT()),
                cereal::make_nvp("humi",getH()),
                cereal::make_nvp("rain",getR()),
                cereal::make_nvp("vel_vent",getVV()),
                cereal::make_nvp("vel_racha",getVR()),
                cereal::make_nvp("dir_vent",getDV()) );
    }
    return ss.str();
}
