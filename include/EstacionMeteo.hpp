#ifndef __ESTACION__METEO__HPP
#define __ESTACION__METEO__HPP

#include <string>

class EstacionMeteo
{
    public:
        bool arranca();
        float getT();
        float getH();
        unsigned getR();
        std::string getcurrent();// TODO (sergio#1#09/10/15): Implementar con cereal JSON
    private:
        float temp;
        float humi;
        int rain;
};
#endif // __ESTACION__METEO__HPP
