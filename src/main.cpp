#include <iostream>
#include <string>

#include "EstacionMeteo.hpp"
#include "Anotador.hpp"
// TODO (sergio#1#18/01/16): - Hacer SocketServer estatica pura ...
//- Convertir namespace EstacionMeteo en clase estatica pura.

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
