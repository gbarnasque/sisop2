#pragma once
#include <netinet/in.h> //sockaddr_in

#define MAX_MSG 129
#define MAX_CLIENTS 1

class Servidor {
    private: 
        int _serverFD;
        struct sockaddr_in serverAddress;
        struct sockaddr_in clientAddress;
        char buffer[MAX_MSG];
        int _port;

    public:
        Servidor(char* port);
        void start();
        void info();

        static bool checkStartupParameters(int argc, char** argv);
        static void help();
};