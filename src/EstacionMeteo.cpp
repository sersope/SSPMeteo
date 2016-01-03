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
#include <curl/curl.h>
#include <math.h>

EstacionMeteo::EstacionMeteo()
{
    temp = -70.0;
    humi = 0.0;
    rain = 0;
    rain_init = 0;
    vel_vent = 0.0;
    vel_racha = 0.0;
    dir_vent = 0;

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
    Anotador log("estacionMeteo.log");
    double un_minuto = 1 * 60.0;                // segundos
    double periodo_salvadatos = 1 * 60.0;      // segundos// NOTE (sergio#1#03/01/16): Para pruebas ponemos 1 minuto, luego ya se verá
    time_t timer_un_minuto,timer_salvadatos,ahora;
    bool datosOK = false;
    // Espera a que lleguen datos validos por primera vez
    log.anota("Esperando datos válidos...");
    while(!datosOK)
    {
        for( int i = 0; i < 6; i++)
            if( esMensajeBueno(i))
                datosOK = true;
            else
                datosOK = false;
    }
    log.anota("Datos válidos OK.");
    // Inicializacion de lluvia
    rain_init = getR();
    rain_cola.push(rain_init);
    // Inicializacion de tiempos
    time(&ahora);
    timer_un_minuto = ahora;
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
            // Resetea la lluvia diaria
            rain_init = getR();

            ayer = hoy;
        }
        if(difftime(ahora,timer_un_minuto) >= un_minuto)
        {
            actualizaRH(); // Llamar cada minuto
            // Envio a weather underground
            uploadWunder();

            timer_un_minuto = ahora;
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
    if( unit == 'F')
        return (1.8 * temp + 32); // Convierte a ºF
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
        return (1.8 * trocio + 32);
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
    rain_dia = getR() - rain_init;
    if( unit == 'I')
        return (rain_dia / 25.4 ); // A pulgadas
    else
        return rain_dia;
}

void EstacionMeteo::actualizaRH()
{
    rain_cola.push(getR());
    rain_hora = rain_cola.back() - rain_cola.front();
    if(rain_cola.size() > 60) // 1 hora = 60 minutos. La funcion se ha de llamar cada minuto (necesario)
        rain_cola.pop();
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
            "ID=ICOMUNID54&PASSWORD=laura11&action=updateraw&dateutc=now&tempf=%f&humidity=%f&dewptf=%f&dailyrainin=%f&rainin=%f&windspeedmph=%f&windgustmph=%f&winddir=%d",
            getT('F'), getH(), getTR('F'), getRD('I'), getRH(), getVV('M'), getVR('M'), getDV());
        /* Now specify the POST data */
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postfield);
        log.anota(postfield);
        /* Perform the request, res will get the return code */
        res = curl_easy_perform(curl);
        /* Check for errors */
        if(res != CURLE_OK)
            log.anota("ERROR 1 en EstacionMeteo::uploadWunder.");

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
