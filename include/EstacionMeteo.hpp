#ifndef __ESTACION__METEO__HPP
#define __ESTACION__METEO__HPP

class EstacionMeteo
{
    public:
        bool arranca();
        float getT();
        float getH();
        unsigned getR();
    private:
        float temp;
        float humi;
        int rain;
};
#endif // __ESTACION__METEO__HPP
