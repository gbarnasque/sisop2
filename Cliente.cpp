#include "Cliente.hpp"

Cliente::Cliente(char* serverIp, char* serverPort) {
    _socket = new TCPSocket(serverIp, serverPort);
    _socket->printSocketInfo();

    if(_socket->connectSocket()) {
        StringUtils::printDanger("Problema ao conectar ao servidor");
        exit(3);
    }

    StringUtils::printSuccess("Conectado!");
}

void Cliente::handleExit(){
    StringUtils::printWarning("Saindo do aplicativo cliente");
    memset(sendLine, 0, sizeof(sendLine));
    _socket->sendMessage(sendLine);
    _socket->closeSocket();
    exit(0);
}

void Cliente::interact() {
    StringUtils::printInfo("Esperando pelo input do usuario...");
    while(fgets(sendLine, MAX_MSG, stdin) != NULL) {
        StringUtils::removeNewLineAtEnd(sendLine);

        _socket->sendMessage(sendLine);

        memset(receiveLine, 0, sizeof(receiveLine));

        if(_socket->receive(receiveLine, MAX_MSG) == -1){
            StringUtils::printDanger("O servidor encerrou a conexão");
            exit(4);
        }
        
        StringUtils::printInfo("Mensagem recebida do servidor:");
        puts(receiveLine);

        memset(sendLine, 0, sizeof(sendLine));
        StringUtils::printInfo("Esperando pelo input do usuario...");
    }
    handleExit();
}

void Cliente::help() {
    std::cout << std::endl;
    StringUtils::printBold("USAGE");
    puts("./app_cliente [-h | --help] <PERFIL> <IP_SERVIDOR> <PORTA>");
    puts("Onde <PERFIL> deve começar com @ e conter entre 3 e 20 caracteres");
    puts("<IP_SERVIDOR> deve ser no format x.x.x.x, onde x esta no intervalo [0, 255]");
    puts("<PORTA> esta no intervalo [1,65535]");
    std::cout << std::endl;

    StringUtils::printBold("EXAMPLES");
    puts("./app_cliente @teste 0.0.0.0 8080");
    puts("./app_cliente @programa 127.0.0.1 8000");
    puts("./app_cliente -h");
    puts("./app_cliente --help");
    std::cout << std::endl;

    StringUtils::printBold("DESCRIPTION");
    std::cout << "Programa para criacao de um programa cliente para atender os requisitos da parte 1" << std::endl << "do trabalho pratico da cadeira de Sistemas Operacionais II" << std::endl;
}

bool Cliente::checkStartupParameters(int argc, char** argv) {
    int port;

    if(argc < 4) {
        StringUtils::printDanger("Quantidade de parametros passados eh insuficiente");
        return false;
    }

    if((!strcmp(argv[1], "--help") || !strcmp(argv[1], "-h"))) 
        return false;
    
    std::string perfil(argv[1]);
    if(perfil[0] != '@') {
        StringUtils::printDanger("Seu perfil precisa comecar com @");
        return false;
    }
    if(perfil.length() < 4 || perfil.length() > 20) {
        StringUtils::printDanger("Seu perfil precisa conter entre 4 e 20 caracteres");
        return false;
    }
     
    std::string ip(argv[2]);
    std::size_t found;
    std::string s_ip;
    int periodCount = 0;
    while((found = ip.find_first_of('.')) != std::string::npos) {
        s_ip = ip.substr(0, found);
        try{
            stoi(s_ip);
        }
        catch (const std::invalid_argument&) {
            StringUtils::printDanger("IP_SERVIDOR deve estar no formato x.x.x.x, onde x esta no intervalo [0, 255]");
            return false;
        }
        ip = ip.substr(found+1);
        periodCount++;
    }
    if(periodCount != 3) {
        StringUtils::printDanger("IP_SERVIDOR deve estar no formato x.x.x.x, onde x esta no intervalo [0, 255]");
        return false;
    }
    try{
        stoi(ip);
    }
    catch (const std::invalid_argument&) {
        StringUtils::printDanger("IP_SERVIDOR deve estar no formato x.x.x.x, onde x esta no intervalo [0, 255]");
        return false;
    }

    std::string s_port(argv[3]);
    port = std::stoi(s_port);
    if(port < 1 || port > 65535) {  // Check for valid PORT
        StringUtils::printDanger("Porta fora do intervalo permitido [1, 65535]");
        return false;
    }

    return true;
}