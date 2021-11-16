#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <iostream>
#include <semaphore.h>

#include "../TCPSocket/TCPSocket.hpp"
#include "../models/Pacote.hpp"
#include "../StringUtils/StringUtils.hpp"
#include "../models/Perfil.hpp"

#define MAX_MSG 5*1024
#define MAX_TWEET_LEN 128
#define MIN_USER_LEN 4
#define MAX_USER_LEN 20

#define MAX_RETRIES 5
#define TIME_TO_RETRY 2000*1000 // microssegundos = milissegundos*1000

class FrontEnd  {
    private:
        TCPSocket* _socket;
        std::string _usuario;
        std::vector<Pacote> _pacotes;

        sem_t _semaphorSendPacote;
        bool _serverDown;
        int _triesToConnect;

    public:
        FrontEnd(char* serverIp, char* serverPort, char* user);

        bool connectToServer(const char* serverIp, const char* serverPort);

        void receiveNotifications();
        void handleExit();
        void sendPacote(Pacote pacote);
        void sendPacote(Comando comando, std::string payload);
        void sendPacotes();
};