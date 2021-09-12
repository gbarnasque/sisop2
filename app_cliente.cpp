#include <iostream>
#include <signal.h> //signal

#include "Cliente.hpp"

Cliente* Signalhandler;

void handleSignal(int sigNum) {
    //StringUtils::print("Entered signal handler", YELLOW_TEXT);
    Signalhandler->handleExit();
}

int main(int argc, char** argv) {
    if(!Cliente::checkStartupParameters(argc, argv)) {
        Cliente::help();
        exit(0);
    }

    signal(SIGINT, handleSignal);
    
    char* ip = argv[2];
    char* port = argv[3];

    Cliente* c = new Cliente(ip, port);
    Signalhandler = c;
    
    c->interact();

    return 1;
}