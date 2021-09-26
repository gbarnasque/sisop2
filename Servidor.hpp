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

typedef void * (*THREADFUNCPTR)(void *);

#define MAX_MSG 1024
#define MAX_CLIENTS 5

struct notificacao {
    int id;
    time_t timestamp; // Momento em que a notificacao foi recebida
    std::string message;
    int tamanho;
    int quantiadadeSeguidoresAReceber;
};

struct perfil {
    std::string autor;
    std::vector<std::string> seguidores;
    std::vector<notificacao> notificacoesRecebidas;
    std::vector<std::pair<std::string,int>> notificacoesPendentes; // <Perfil a receber, id notificacao>
    std::vector<int> socketDescriptors;
};

class Servidor {
    private: 
        int _currentClientFD;
        TCPSocket* _serverSocket;
        sem_t semaphorClientFD;
        std::vector<perfil> perfis;

    public:
        Servidor(char* port);
        void start();
        void info();
        void handleClient();
        
        static void* handleClientStatic(void* context);
        static bool checkStartupParameters(int argc, char** argv);
        static void help();
};