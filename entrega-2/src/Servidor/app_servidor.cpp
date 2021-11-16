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

    Servidor* s = NULL;
    bool primary = false;
    if(argc == 2) {
        s = new Servidor(argv[1]);
        primary = true;
    }
    else if (argc == 4)
        s = new Servidor(argv[1], argv[2], argv[3]);
    
    Signalhandler = s;
    
    s->info();
    
    s->start();

    return 1;
}