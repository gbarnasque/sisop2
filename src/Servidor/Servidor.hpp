#pragma once

#include <iostream>
#include <string>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <ctime>
#include <vector>
#include <utility> // pair make_pair
#include <fstream>

#include "../StringUtils/StringUtils.hpp"
#include "../TCPSocket/TCPSocket.hpp"
#include "../models/Pacote.hpp"
#include "../models/Perfil.hpp"
#include "../models/Notificacao.hpp"

#define DEFAULT_IP "0.0.0.0" 

typedef void * (*THREADFUNCPTR)(void *);

#define MAX_MSG 1024
#define MAX_CLIENTS 5


class Servidor {
    private: 
        int _currentClientFD;
        TCPSocket* _serverSocket;
        sem_t _semaphorClientFD;
        sem_t _semaphorNotifications;
        sem_t _semaphorPerfisInclusion;
        std::vector<Perfil> _perfis;
        int _GLOBAL_NOTIFICACAO_ID;
        std::string _saveFileName;

        bool _isPrimary;
        TCPSocket* _primaryServerSocket;
        vector<pair<pid_t, int>> _poolServidores; // PID e FD do Servidor
        sem_t _semaphorPool;

        void saveFile();
        void fillFromFile();
        void notifyAllConnectedClients();
        void sendNotificacao(std::string from, int idNotificacao);
        void sendNotificacoes(Perfil to);

        void notifyClientsToUpdateServer();
        void sendPacoteToAllClients(Pacote p);

    public:
        Servidor(char* port, char* primaryServerPort);
        void start();
        void info();
        void handleClient();

        Pacote handleConnect(std::string usuario, int socketDescriptor);
        void handleDisconnect(std::string usuario, int socketDescriptor);
        Pacote handleSend(std::string usuario, time_t timestamp, std::string payload, int tamanhoPayload);
        Pacote handleFollow(std::string usuarioSeguido, std::string usuarioSeguidor);

        Perfil getPerfilByUsername(string username);
        static void* handleClientStatic(void* context);
        static bool checkStartupParameters(int argc, char** argv);
        static void help();
        void printPerfis();

        void gracefullShutDown();

        static void* handleNotificationsStatic(void* context);
        void handleNotifications();

        static void* handleHealthCheckStatic(void* context);
        void handleHealthCheck();

        void updatePool(std::string serverPID, std::string serverFD);
        void notifyServersToUpdatePool(std::string serverPID, std::string serverFD);
        void sendPacoteToAllServers(Pacote p);

        static void* handleServerNotificationsStatic(void* context);
        void handleServerNotifications();

        void printPool();
};