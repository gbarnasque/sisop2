#include <iostream>
#include "Servidor.hpp"

int main(int argc, char** argv) { 
    if(!Servidor::checkStartupParameters(argc, argv)) {
        Servidor::help();
        exit(0);
    }

    Servidor s(argv[1]);
    s.info();
    s.start();

    return 1;
}