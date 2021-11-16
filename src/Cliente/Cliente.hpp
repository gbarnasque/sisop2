#pragma once
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

#include "./FrontEnd.hpp"

class Cliente {
    private:
        char sendLine[MAX_MSG], receiveLine[MAX_MSG];
        TCPSocket* _socket;
        std::string _usuario;
        std::vector<Pacote> _pacotes;

        FrontEnd* _frontEnd;

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