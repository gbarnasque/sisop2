#include <iostream>
#include <signal.h> //signal

#include "Servidor.hpp"

Servidor* Signalhandler;

void handleSignal(int sigNum) {
    Signalhandler->gracefullShutDown();
}

int main(int argc, char** argv) { 
    if(!Servidor::checkStartupParameters(argc, argv)) {
        Servidor::help();
        exit(0);
    }

    signal(SIGINT, handleSignal);

    Servidor* s;
    char defaultPort[2] = "0";
    if(argc == 2)
        s = new Servidor(argv[1], defaultPort);
    else if(argc == 3)
        s = new Servidor(argv[1], argv[2]);
    
    Signalhandler = s;
    
    s->info();
    s->start();

    return 1;
}