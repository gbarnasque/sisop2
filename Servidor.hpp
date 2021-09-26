#pragma once

#include <iostream>
#include <string>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <ctime>
#include <vector>
#include <utility> // pair make_pair

#include "StringUtils.hpp"
#include "TCPSocket.hpp"
#include "Pacote.hpp"
#include "Perfil.hpp"
#include "Notificacao.hpp"

typedef void * (*THREADFUNCPTR)(void *);

#define MAX_MSG 1024
#define MAX_CLIENTS 5


class Servidor {
    private: 
        int _currentClientFD;
        TCPSocket* _serverSocket;
        sem_t semaphorClientFD;
        std::vector<Perfil> perfis;

    public:
        Servidor(char* port);
        void start();
        void info();
        void handleClient();

        void handleConnect(string usuario, int socketDescriptor);
        void handleDisconnect(string usuario, int socketDescriptor);


        
        static void* handleClientStatic(void* context);
        static bool checkStartupParameters(int argc, char** argv);
        static void help();
};