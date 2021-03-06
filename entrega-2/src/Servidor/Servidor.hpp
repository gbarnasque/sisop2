#pragma once

#include <iostream>
#include <string>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <ctime>
#include <vector>
#include <fstream>

#include "../StringUtils/StringUtils.hpp"
#include "../TCPSocket/TCPSocket.hpp"
#include "../models/Pacote.hpp"
#include "../models/Perfil.hpp"
#include "../models/Notificacao.hpp"

typedef void * (*THREADFUNCPTR)(void *);

#define MAX_MSG 5*1024
#define MAX_CLIENTS 20
#define MAX_RETRIES 5
#define TIME_TO_RETRY 500*1000 // microssegundos = milissegundos*1000
#define TIME_TO_RESTART 250*1000

class Servidor {
    private: 
        int _currentFD;
        TCPSocket* _serverSocket;
        sem_t _semaphorCurrentFD;
        sem_t _semaphorNotifications;
        sem_t _semaphorPerfisInclusion;
        std::vector<Perfil> _perfis;
        int _GLOBAL_NOTIFICACAO_ID;
        std::string _saveFileName;

        void saveFile();
        void fillFromFile();
        Pacote sendNotificacao(std::string from, int idNotificacao);
        void sendNotificacoes(Perfil* to);

        TCPSocket* _primaryServerSocket;
        bool _isPrimary;
        std::vector<ServerPerfil> _pool;

        int _currentBackupFD;
        pthread_t _notificationHandler;

        int _poolSize;
        int _minPoolSize;
        int _triesToConnectToPrimary;
        void printPool();
    public:
        Servidor(char* port);
        Servidor(char* port, char* primaryIp, char* primaryPort);
        void start();
        void info();
    
        Pacote handleClienteConnect(std::string usuario, int socketDescriptor);
        void handleDisconnect(std::string usuario, int socketDescriptor);
        Pacote handleSend(std::string usuario, time_t timestamp, std::string payload, int tamanhoPayload);
        Pacote handleFollow(std::string usuarioSeguido, std::string usuarioSeguidor);

        static void* handleClientStatic(void* context);
        void handleClient();
        static bool checkStartupParameters(int argc, char** argv);
        static void help();
        void printPerfis();

        void gracefullShutDown();

        static void* handleNotificationsStatic(void* context);
        void handleNotifications();

        /* Novas funcionalidades para entrega 2*/
        static void* ProcessKeyboardInputStatic(void* context);
        void ProcessKeyboardInput();

        Pacote handleServerConnect(std::string pid, std::string payload, int FD);

        void servidorPrimarioHandler();

        void sendPacoteToAllServidoresBackup(Pacote pacote);
        void restartAsPrimary();
        void resetClientSockets();
        bool election();
        bool connectToPrimary(char* primaryIp, char* primaryPort);
        void resetPool();
};