#pragma once
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string.h>
#include <string>

#include "StringUtils.hpp"
#include "TCPSocket.hpp"
#include "Pacote.hpp"

#define MAX_MSG 1024

class Cliente {
    private:
        char sendLine[MAX_MSG], receiveLine[MAX_MSG];
        TCPSocket* _socket;

    public:
        Cliente(char* serverIp, char* serverPort, char* user);        
        void handleExit(); 

        void interact();

        static void help();
        static bool checkStartupParameters(int argc, char** argv);
};