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

    Servidor* s = new Servidor(argv[1]);
    Signalhandler = s;
    
    s->info();
    s->start();

    return 1;
}