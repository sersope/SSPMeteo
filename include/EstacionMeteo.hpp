#ifndef __ESTACION__METEO__HPP
#define __ESTACION__METEO__HPP

#include <string>
#include <thread>

class EstacionMeteo
{
    public:
        EstacionMeteo();
        ~EstacionMeteo();
        bool arranca();
        void termina();
        float getT();
        float getH();
        float getVV();
        float getVR();
        unsigned getDV();
        unsigned getR();
        std::string getcurrent();
    private:
        float temp;
        float humi;
        int rain;
        float vel_vent; // km/h
        float vel_racha;// km/h
        int dir_vent; //º sexagesimales

        std::thread * pth;
        void procesa();     //the working thread
        bool terminar;
};
#endif // __ESTACION__METEO__HPP
