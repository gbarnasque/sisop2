#include <iostream>
#include <signal.h> //signal
#include <string.h>

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
    
   /* char* ip;
    char* port;
    strcpy(ip, argv[2]);
    strcpy(port, argv[3]);
*/
    std::cout << argv[2] << ":" << argv[3] << std::endl;
    Cliente* c = new Cliente(argv[2], argv[3], argv[1]);
    Signalhandler = c;
    
    c->interact();

    return 1;
}