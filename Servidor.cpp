#include <iostream>
#include <string>

#include <cstdio>
#include <sys/socket.h>
#include <cstdlib>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h> // inet functions
#include <unistd.h> //fork, close
#include <pthread.h>

#include "Servidor.hpp"
#include "StringUtils.hpp"


// https://pubs.opengroup.org/onlinepubs/7908799/xns/syssocket.h.html
// https://www.cs.dartmouth.edu/~campbell/cs60/socketprogramming.html


Servidor::Servidor(char* port) {
    _port = atoi(port);
    _serverFD = socket(AF_INET, SOCK_STREAM, 0);

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddress.sin_port = htons(_port);

    bind(_serverFD, (struct sockaddr*) &serverAddress, sizeof(serverAddress));

    listen(_serverFD, MAX_CLIENTS);

}

void Servidor::start() {
    socklen_t clientLen;
    int connFD, n;

    StringUtils::printSuccess("Servidor iniciado");
    while(true) {
        /*clientLen = sizeof(clientAddress);
        connFD = accept(serverFD, (struct sockaddr*) &client_address, &clientLen);
        printf("Request received\n");

        while( (n = recv(connFD, buffer, MAX_MSG, 0)) > 0 ) {
            printf("String received from and resent to client: ");
            puts(buffer);
            send(connFD, buffer, n, 0);
            memset(buffer, 0, sizeof(buffer));
        }

        if (n < 0 ) {
            perror("Read error");
            exit(1);
        }
        close(connFD);*/


        clientLen = sizeof(clientAddress);
        StringUtils::printInfo("Esperando algum cliente conectar... ");
        _connFD = accept(_serverFD, (struct sockaddr *) &clientAddress, &clientLen);
        if(_connFD == -1) {
            StringUtils::printDanger("Houve um problema ao conectar com o cliente");
            continue;
        }

        StringUtils::printInfo("Cliente conectado...");


        pthread_t clientHandlerThread;
        if (pthread_create(&clientHandlerThread, NULL, &Servidor::handleClientStatic, this) != 0) { // Static func of class, this is necessary to keep the context of the class
        //if (pthread_create(&clientHandlerThread, NULL, (THREADFUNCPTR) &Servidor::handleClient, this) == 0) { // Pointer to func
            StringUtils::printDanger("Erro ao criar a Thread para lidar com o cliente.");
        }
    }
    
    close(_serverFD);
}

void* Servidor::handleClientStatic(void* context) {
    ((Servidor *)context)->handleClient();
    pthread_exit(NULL);
}

void Servidor::handleClient() {
    int connFD = _connFD;
    StringUtils::printInfo("Entered thread");
    char buffer[MAX_MSG];
    int n;
    StringUtils::printWarning("Outra Thread criada para lidar com as requisicoes do cliente");

    //close listening socket to stop bugs
    //close(_serverFD);

    memset(buffer, 0, sizeof(buffer));
    while ( (n = recv(connFD, buffer, MAX_MSG, 0)) > 0)  { // Read the message and save in the buffer
        StringUtils::printInfo("Mensagem recebida e enviada de volta ao cliente: ");
        std::cout << buffer << std::endl;
        send(connFD, buffer, n, 0);
        memset(buffer, 0, sizeof(buffer));
    }

    close(connFD);

    if (n < 0)
        StringUtils::printDanger("Houve um problema ao ler a mensagem do cliente");
    else if (n == 0)
        StringUtils::printWarning("Cliente se desconectou do servidor");
    //pthread_exit(0);
}


void Servidor::info() {
    char ip[INET_ADDRSTRLEN];
    struct in_addr ipAddr = (&serverAddress)->sin_addr;
    inet_ntop(AF_INET, &ipAddr, ip, INET_ADDRSTRLEN);
    std::string ip_str(ip);
    std::string info = "Servidor rodando em " + ip_str + ":" + to_string(_port);
    StringUtils::printSuccess(info);
}

bool Servidor::checkStartupParameters(int argc, char** argv) {
    int port;
    if(argc < 2) {
        StringUtils::printDanger("Quantidade de parametros passados eh insuficiente");
        return false;
    }

    if((!strcmp(argv[1], "--help") || !strcmp(argv[1], "-h"))) 
        return false;

    std::string s(argv[1]);
    if(s.find_first_not_of("0123456789") != std::string::npos) {
        StringUtils::printDanger("Porta contem caracteres nao numericos");
        return false;
    }

    port = stoi(s);
    if(port < 1 || port > 65535) {  // Check for valid PORT
        StringUtils::printDanger("Porta fora do intervalo permitido [1, 65535]");
        return false;
    }
    
    return true;
}

void Servidor::help() {
    std::cout << std::endl;
    StringUtils::printBold("USAGE");
    puts("./app_servidor [-h | --help] <PORTA>");
    puts("Onde <PORTA> esta no intervalo [1, 65535]");
    std::cout << std::endl;

    StringUtils::printBold("EXAMPLES");
    puts("./app_servidor 8080");
    puts("./app_servidor 8888");
    puts("./app_servidor -h");
    puts("./app_servidor --help");
    std::cout << std::endl;

    StringUtils::printBold("DESCRIPTION");
    std::cout << "Programa para criacao de um servidor para atender os requisitos da parte 1" << std::endl << "do trabalho pratico da cadeira de Sistemas Operacionais II" << std::endl;
}