/**
   Codificación de datos:
   -   nº del mensaje [1..255] 8 bits
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
#include "Anotador.hpp"
#include "BMP085.hpp"

#include <sstream>
#include <ctime>
#include <thread>
#include <curl/curl.h>
#include <math.h>
#include <queue>
#include <fstream>

namespace EstacionMeteo
{
    // Variables internas al namespace
    float temp = -70.0;             //celsisus o faranheit
    float humi = 0.0;
    float trocio = 0.0;             // Temperatura de rocio (calculada)
    float rain = 0.0;               // Lluvia actual (acumulada en el arduino)
    float rain_init = 0.0;          // Lluvia al inicio del programa y al inicio del dia
    float rain_dia = 0.0;           // Lluvia acumulada en el dia
    float rain_hora = 0.0;          // Lluvia acumulada en la ultima hora, se gestiona con una cola
    float vel_vent = 0.0;           // km/h o mph
    float vel_racha = 0.0;          // km/h o mph
    int dir_vent = 0;               // º sexagesimales
    float tempIn = 0.0;             // Temperatura interior
    float tempInF = 0.0;            // Temperatura interior en *F
    float pres = 0.0;               // Presión relativa
    float presInch = 0.0;           // Presión relativa en pulgadas de mercurio
    std::queue<float> rain_cola;    //Cola para la lluvia en la ultima hora
    std::thread * pth = nullptr;
    bool terminar = false;
    BMP085 bmp(BMP085::OSS_ULTRAHIGH); // Sensor de presion y temperatura interior

    // Funciones internas al namespace
    void procesa();                 //the working thread
    float getT(char unit='m');      // unit = 'm' sistema metrico
    float getH();
    float getTR(char unit='m');
    float getVV(char unit='m');
    float getVR(char unit='m');
    unsigned getDV();
    float getR(char unit='m');
    float getRD(char unit='m'); //Obten la lluvia diaria (float porque puede ser en inches)
    void actualizaRH(); // Actualiza la cola para la lluvia de la ultima hora
    float getRH(char unit='m'); //Obten la lluvia en la ultima hora (float porque puede ser en inches)
    void actualizaPR();  //Obten la presion relativa
    void actualizaTI();  // Obten la temperatura interior (el sensor esta dentro de casa)
    bool uploadWunder();
    bool esMensajeBueno(int nmen);
}

bool EstacionMeteo::arranca()
{
    terminar = false;
    pth = new std::thread(&EstacionMeteo::procesa);
    return ReceptorRF433::arranca();
}

void EstacionMeteo::termina()
{
    Anotador log("sspmeteo.log");
    if(pth != 0)
    {
        terminar = true;
        pth->join();
        log.anota("estacionMeteo: Proceso terminado.");
    }
}

void EstacionMeteo::procesa()
{
    Anotador datos_log("datos.dat");
    Anotador log("sspmeteo.log");
    double un_minuto = 1 * 60.0;               // segundos
    double periodo_salvadatos = 5 * 60.0;      // segundos
    time_t timer_un_minuto,timer_salvadatos,ahora;
    int hoy;
    int mes;
    int anyo;
    int ayer;
    char nomfile[30];
    bool primera_vez = true;
    // Espera a que lleguen datos validos por primera vez desde el receptor RF
    log.anota("estacionMeteo: Esperando datos válidos...");
    int todosOK = 0;
    // FIXME (sergio#1#16/01/16): En este bucle el programa no puede terminar
    while(todosOK < 6)
    {
        todosOK = 0;
        for( int i = 0; i < 6; i++)
            if( esMensajeBueno(i))
                todosOK++;
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    log.anota("estacionMeteo: Datos válidos OK.");
    // Bucle de proceso
    while(!terminar)
    {
        time(&ahora);
        hoy = localtime(&ahora)->tm_mday;
        if (primera_vez)
        {
            // Inicializacion de tiempos
            timer_un_minuto = ahora;
            timer_salvadatos = ahora;
            mes = localtime(&ahora)->tm_mon + 1;
            anyo = localtime(&ahora)->tm_year + 1900;
            ayer = hoy;
            //Genera el nombre del fichero de datos diarios
            sprintf(nomfile, "%d-%02d-%02d.dat", anyo, mes, hoy);
            datos_log.setName(std::string(nomfile));
            // Inicializacion de lluvia diaria
            // El valor de lluvia diaria inicial se toma del fichero que se salva al inicio del dia
            // salvo si el Arduino se ha reiniciado.
            float aux_getR = getR();
            std::ifstream file("lluvia.dat");
            if( file.fail())
                rain_init = aux_getR;
            else
            {
                    float aux_fic;
                    file >> aux_fic;
                    file.close();
                    if( aux_fic > aux_getR ) // Esto indica que el arduino se ha reiniciado.
                        rain_init = aux_getR;
                    else
                        rain_init = aux_fic;
            }
            // Envio y salvado de los primeros datos
            actualizaRH();
            // Actualiza los valores del sensor BMP180
            actualizaPR();
            actualizaTI();

            datos_log.anota(getcurrent());
            uploadWunder();
            primera_vez = false;
        }
        // Por cambio de dia
        if(hoy != ayer)
        {
            // Obten nuevo nombre para el fichero de datos diarios
            mes = localtime(&ahora)->tm_mon + 1;
            anyo = localtime(&ahora)->tm_year + 1900;
            sprintf(nomfile, "%d-%02d-%02d.dat", anyo, mes, hoy);
            datos_log.setName(std::string(nomfile));
            // Resetea la lluvia diaria
            rain_init = getR();
            // Salva el valor de lluvia inicial del dia
            std::ofstream file("lluvia.dat", std::ofstream::trunc);
            file << rain_init << std::endl;
            file.flush();
            file.close();

            ayer = hoy;
        }
        if(difftime(ahora,timer_un_minuto) >= un_minuto)
        {
            actualizaRH(); // Llamar cada minuto
            // Actualiza los valores del sensor BMP180
            actualizaPR();
            actualizaTI();

            timer_un_minuto = ahora;
        }
        if(difftime(ahora,timer_salvadatos) >= periodo_salvadatos)
        {
            // Salva datos actuales a fichero
            datos_log.anota(getcurrent());
            // Envio a weather underground
            uploadWunder();
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
    if( unit == 'F')
        return (1.8 * temp + 32.0); // Convierte a ºF
    else
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

float EstacionMeteo::getTR(char unit)
{
    trocio = pow(humi / 100.0, 1.0 / 8.0) * (112.0 + 0.9 * temp) + 0.1 * temp - 112.0;
    if( unit == 'F' )
        return (1.8 * trocio + 32.0);
    else
        return trocio;
}

float EstacionMeteo::getR(char unit)
{
    if( esMensajeBueno(2) )
    {
        unsigned aux;
        aux = ReceptorRF433::mensaje_tipo[2][0] & 0xFFFF;
        if( aux >= 0 && aux <= 65535 )
            rain = aux * 0.138; // Factor mm/tick del pluviometro
    }
    if( unit == 'I')
        return (rain / 25.4);
    else
        return rain;
}

float EstacionMeteo::getRD(char unit)
{
    float aux = getR() - rain_init;
    if ( aux >= 0.0 )
        rain_dia = aux;
    if( unit == 'I')
        return (rain_dia / 25.4 ); // A pulgadas
    else
        return rain_dia;
}

// Almacena en una cola los valores de lluvia de la ultima hora.
// La diferencia entre el primer valor de la cola y el ultimo es la lluvia caída en la hora.
void EstacionMeteo::actualizaRH()
{
    float aux_getR = getR();
    rain_cola.push(aux_getR);
    rain_hora = rain_cola.back() - rain_cola.front();
    if( rain_cola.size() > 60 ) // 1 hora = 60 minutos. La funcion se ha de llamar cada minuto (necesario)
        rain_cola.pop();

    if( rain_hora < 0.0 ) // Ha habido un reseteo del Arduino. Se debe reiniciar todo
    {
        rain_init = aux_getR;
        while( rain_cola.size() > 1 ) //Vacia la cola a un elemento
            rain_cola.pop();
    }
}

float EstacionMeteo::getRH(char unit)
{
    if( unit == 'I')
        return (rain_hora / 25.4);
    else
        return rain_hora;
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
    if( unit == 'M')
        return  (vel_vent * 0.621371192); // A mph
    else
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
    if( unit == 'M')
        return (vel_racha * 0.621371192); // A mph
    else
        return vel_racha;
}

// Los valores del BMP180  se actualizan en el bucle de proceso y no por peticion de clientes
// Lo contrario podría saturar el sensor y provocar errores en las lecturas
// Los valores convertidos a unidades son para Wunder
// TODO (sergio#1#16/01/16): Tratar el tema  de errores en los valores del sensor BMP180
void EstacionMeteo::actualizaPR()
{
    float p = bmp.getBoth().kPa * 10.0;
    pres = BMP085::getMeanPressure(p, 20);  // 20m. de altitud
    presInch = pres * 0.0295299830714;    // A pulgadas de columna de mercurio
}

void EstacionMeteo::actualizaTI()
{
    tempIn = bmp.getCelcius();
    tempInF = 1.8 * tempIn + 32.0;
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
    ss << getT() << "," << getH() << "," << getTR() << "," << getR() << "," << getRH() << "," << getRD()
       << "," << getVV() << "," << getVR() << "," << getDV() << "," << pres << "," << tempIn
       << "," << ReceptorRF433::mensajes_recibidos;
    for( int i = 0; i < 6; i++)
        ss << "," << ReceptorRF433::mensaje_indice[i];
    return ss.str();
}

bool EstacionMeteo::uploadWunder()
{
    CURL *curl;
    CURLcode res;
    char postfield[255];
    Anotador log("wunder.log");
    /* In windows, this will init the winsock stuff */
    curl_global_init(CURL_GLOBAL_ALL);
    /* get a curl handle */
    curl = curl_easy_init();
    if(curl)
    {
        /* First set the URL that is about to receive our POST. This URL can
           just as well be a https:// URL if that is what should receive the
           data. */
        curl_easy_setopt(curl, CURLOPT_URL, "http://weatherstation.wunderground.com/weatherstation/updateweatherstation.php");
        // Rellenamos los datos a enviar
        sprintf(postfield,
                "ID=ICOMUNID54&PASSWORD=laura11&action=updateraw&dateutc=now&tempf=%f&humidity=%f&dewptf=%f&dailyrainin=%f&rainin=%f&windspeedmph=%f&windgustmph=%f&winddir=%d&baromin=%f&indoortempf=%f",
                getT('F'), getH(), getTR('F'), getRD('I'), getRH('I'), getVV('M'), getVR('M'), getDV(), presInch, tempInF);
        /* Now specify the POST data */
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postfield);
        /* Perform the request, res will get the return code */
        res = curl_easy_perform(curl);
        /* Check for errors */
        if(res != CURLE_OK)
            log.anota("ERROR 1 en EstacionMeteo::uploadWunder.");
        else
            log.anota(postfield);
        /* always cleanup */
        curl_easy_cleanup(curl);
    }
    else
        log.anota("ERROR 2 en EstacionMeteo::uploadWunder.");
    curl_global_cleanup();
    return true;
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
