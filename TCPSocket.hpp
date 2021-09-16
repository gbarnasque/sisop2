#pragma once

#include <stdlib.h>
#include <sys/socket.h> // bind, socket, listen, accept, connect
#include <netinet/in.h> //sockaddr_in
#include <arpa/inet.h> // inet functions
#include <unistd.h> // close
#include <string.h>

#include "StringUtils.hpp"

#define DEFAULT_PORT 8080

class TCPSocket {
    private:
        int _serverFD;
        int _clientFD;
        int _port;
        struct sockaddr_in _serverAddress;
        struct sockaddr_in _clientAddress;

    public:
        TCPSocket();
        TCPSocket(char* serverIp, char* serverPort);
        bool bindServer();
        bool startListen(int backlog);
        int acceptConnection();
        bool connectSocket();
        int sendMessage(char* message);
        int sendMessage(int clientFD, char* message);
        int receive(char* buffer, int maxMessageSize);
        int receive(int clientFD, char* buffer, int maxMessageSize);
        bool closeSocket(); 
        bool closeSocket(int clientFD);

        void printSocketInfo();
};