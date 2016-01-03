#ifndef __ESTACION__METEO__HPP
#define __ESTACION__METEO__HPP

#include <string>
#include <thread>
#include <queue>

class EstacionMeteo
{
    public:
        EstacionMeteo();
        ~EstacionMeteo();
        bool arranca();
        void termina();
        float getT(char unit='m');  // unit = 'm' sistema metrico
        float getH();
        float getTR(char unit='m');
        float getVV(char unit='m');
        float getVR(char unit='m');
        unsigned getDV();
        float getR(char unit='m');
        float getRD(char unit='m'); //Obten la lluvia diaria (float porque puede ser en inches)
        float getRH(char unit='m'); //Obten la lluvia en la ultima hora (float porque puede ser en inches)
        std::string getcurrent();
    private:
        float temp; //celsisus o faranheit
        float humi;
        float trocio;       // Temperatura de rocio (calculada)
        float rain;         // Lluvia actual (acumulada en el arduino)
        float rain_init;    // Lluvia al inicio del programa y al inicio del dia
        float rain_dia;     // Lluvia acumulada en el dia
        float rain_hora;    // Lluvia acumulada en la ultima hora, se gestiona con una cola
        float vel_vent;     // km/h o mph
        float vel_racha;    // km/h o mph
        int dir_vent;       //º sexagesimales
        std::queue<float> rain_cola; //Cola para la lluvia en la ultima hora
        void actualizaRH(); // Actualiza la cola para la lluvia de la ultima hora
        std::thread * pth;
        void procesa();     //the working thread
        bool esMensajeBueno(int nmen);
        bool terminar;
        bool uploadWunder();
};
#endif // __ESTACION__METEO__HPP
