#pragma once
#include <netinet/in.h> //sockaddr_in
#include <string>

#define MAX_MSG 129

class Cliente {
    private:
        int sockFD;
        struct sockaddr_in serverAddr;
        //std::string sendLine;
        //std::string receiveLine;
        char sendLine[MAX_MSG], receiveLine[MAX_MSG];

    public:
        Cliente(char* serverIp, char* serverPort);        
        void handleExit(); 

        void interact();

        static void help();
        static bool checkStartupParameters(int argc, char** argv);
};