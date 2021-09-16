#pragma once

#include <iostream>
#include <string>
#include <string.h>
#include <pthread.h>

#include "StringUtils.hpp"
#include "TCPSocket.hpp"

typedef void * (*THREADFUNCPTR)(void *);

#define MAX_MSG 129
#define MAX_CLIENTS 2

class Servidor {
    private: 
        int _connFD;
        TCPSocket* _serverSocket;

    public:
        Servidor(char* port);
        void start();
        void info();
        void handleClient();
        
        static void* handleClientStatic(void* context);
        static bool checkStartupParameters(int argc, char** argv);
        static void help();
};