#ifndef ESTACION_METEO_HPP
#define ESTACION_METEO_HPP

#include "ReceptorRF433.hpp"
#include <thread>

// TODO (sergio#1#23/09/15): Añadir control de estado y errores de la clase.
const int N_MENSAJES = 3;   // Número total de mensajes
const int RF_PIN = 2;

struct DatosMeteo
{
    float temp;
    float humi;
    int rain;
};

class EstacionMeteo
{
    public:
        EstacionMeteo();
        virtual ~EstacionMeteo();
        bool arranca();
        bool para();
        float getT(){ return datos.temp; };
        float getH(){ return datos.humi; };
        int getR(){ return datos.rain; };
    private:
        DatosMeteo datos;
        ReceptorRF433 receptor;
        std::thread *philo;
        int mensaje;
        int mensaje_ant;
        bool parar;
        void procesa();
};
#endif // ESTACION_METEO_HPP
