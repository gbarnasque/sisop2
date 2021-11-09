#pragma once

#include <stdlib.h>
#include <sys/socket.h> // bind, socket, listen, accept, connect
#include <netinet/in.h> //sockaddr_in
#include <arpa/inet.h> // inet functions
#include <unistd.h> // close
#include <string.h>

#include "../StringUtils/StringUtils.hpp"

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
        TCPSocket(const char* serverIp, const char* serverPort);
        bool bindServer();
        bool startListen(int backlog);
        int acceptConnection();
        bool connectSocket();
        int sendMessage(char* message);
        int sendMessage(const char* message); 
        int sendMessage(int FD, char* message);
        int sendMessage(int FD, const char* message);
        int receive(char* buffer, int maxMessageSize);
        int receive(int FD, char* buffer, int maxMessageSize);
        bool closeSocket(); 
        bool closeSocket(int FD);

        void printSocketInfo();
};