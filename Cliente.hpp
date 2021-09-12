#pragma once
#include <netinet/in.h> //sockaddr_in

#define MAX_MSG 129

class Cliente {
    private:
        int sockFD;
        struct sockaddr_in serverAddr;
        char sendLine[MAX_MSG], receiveLine[MAX_MSG];

    public:
        Cliente(char* serverIp, char* serverPort);        
        void handleExit(); 

        void interact();

        static void help();
        static bool checkStartupParameters(int argc, char** argv);
};