#include <iostream>
#include <signal.h> //signal

#include "Cliente.hpp"

Cliente* Signalhandler;

void handleSignal(int sigNum) {
    Signalhandler->handleExit();
}

int main(int argc, char** argv) {
    if(!Cliente::checkStartupParameters(argc, argv)) {
        Cliente::help();
        exit(0);
    }
    
    signal(SIGINT, handleSignal);

    Cliente* c = new Cliente(argv[2], argv[3], argv[1]);
    Signalhandler = c;
    
    c->interact();

    return 1;
}