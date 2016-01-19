/** \mainpage notitle
 * Esta aplicación se usa para la gestión de la estación meteorológica SSPMeteo.
 * La estación consta de un Arduino UNO (estación) al que están conectados los sensores
 * de temperatura, humedad, lluvia, veleta y anemómetro. El Arduino usa un emisor de radiofrecuencia en la banda de los
 * 433 Mhz para enviar los datos. Los datos se reciben en una Raspberry Pi que lleva conectado un receptor de radiofrecuencia de 433 Mhz.
 * En la Raspberry Pi se ejecuta esta aplicación, que procesa y prepara los datos recibidos en valores válidos, de la misma manera que calcula otros.
 * Así mismo, la Raspberry Pi lleva conectado un sensor que lee la presión atmosférica relativa y la temperatura interior de la casa.
 * Todos los datos obtenidos se salvan a ficheros diarios y se envían a Weather Underground en cada periodo de tiempo definido.
 * La aplicación, a su vez, crea un socket de escucha al que pueden conectarse distintos clientes para obtener los datos actuales
 * de la estación.
 * \version 1.0
 * \author Sergio Soriano Peiró
 * \date Enero de 2016
 */
#include <iostream>
#include <string>

#include "EstacionMeteo.hpp"
#include "Anotador.hpp"

int main(int argc, char *argv[])
{
    Anotador log("sspmeteo.log");

    log.anota("+++ NUEVO ARRANQUE +++");
    if (EstacionMeteo::arranca())
        EstacionMeteo::procesa();
    EstacionMeteo::termina();
    log.anota("--- FIN  ---");
    return 0;
}
