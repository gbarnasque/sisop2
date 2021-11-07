#pragma once
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string.h>
#include <string>
#include <pthread.h>
#include <vector>

#include "../StringUtils/StringUtils.hpp"
#include "../TCPSocket/TCPSocket.hpp"
#include "../models/Pacote.hpp"

#define MAX_MSG 1024
#define MAX_TWEET_LEN 128
#define MIN_USER_LEN 4
#define MAX_USER_LEN 20

class Cliente {
    private:
        char sendLine[MAX_MSG], receiveLine[MAX_MSG];
        TCPSocket* _socket;
        std::string _usuario;
        std::vector<Pacote> _pacotes;

    public:
        Cliente(char* serverIp, char* serverPort, char* user);        
        void handleExit(); 

        void interact();
        void ProcessKeyboardInput();
        void receiveNotifications();

        static void* ProcessKeyboardInputStatic(void* context);
        static void* receiveNotificationsStatic(void* context);
        
        bool lineEstaOK(std::string line, Comando c);

        static void help();
        static bool checkStartupParameters(int argc, char** argv);
        
        static void* receiveMessagesOtherServersStatic(void* context);
        void receiveMessagesOtherServers();
};