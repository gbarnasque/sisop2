#include "TCPSocket.hpp"

TCPSocket::TCPSocket() {
    StringUtils::printInfo("Criando socket...1");
    if ((_serverFD = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        StringUtils::printDanger("Problema ao criar o socket!");
        exit(2);
    }

    _port = DEFAULT_PORT;

    memset(&_serverAddress, 0, sizeof(_serverAddress));
    _serverAddress.sin_family = AF_INET;
    _serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    _serverAddress.sin_port = htons(_port);

    StringUtils::printSuccess("Socket criada com sucesso");
}

TCPSocket::TCPSocket(char* serverIp, char* serverPort) {
    // https://man7.org/linux/man-pages/man2/socket.2.html
    // On success, a file descriptor for the new socket is returned. On error, -1 is returned
    
    StringUtils::printInfo("Criando socket...2");
    if ((_serverFD = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        StringUtils::printDanger("Problema ao criar o socket!");
        exit(2);
    }

    _port = atoi(serverPort);

    memset(&_serverAddress, 0, sizeof(_serverAddress));
    _serverAddress.sin_family = AF_INET;
    _serverAddress.sin_addr.s_addr = (serverIp != NULL) ? htonl(inet_addr(serverIp)) : htonl(INADDR_ANY);
    _serverAddress.sin_port = htons(_port);
    StringUtils::printSuccess("Socket criada com sucesso");
}

bool TCPSocket::bindServer() {
    // https://man7.org/linux/man-pages/man2/bind.2.html
    // On success, zero is returned.  On error, -1 is returned
    return (bind(_serverFD, (struct sockaddr *) &_serverAddress, sizeof(_serverAddress)) == 0);
}

// https://man7.org/linux/man-pages/man2/listen.2.html
// On success, zero is returned.  On error, -1 is returned
bool TCPSocket::startListen(int backlog) {
    return (listen(_serverFD, backlog) == 0);
}

int TCPSocket::acceptConnection() {
    socklen_t clientLen = sizeof(_clientAddress);
    return accept(_serverFD, (struct sockaddr *) &_clientAddress, &clientLen);
}

// https://man7.org/linux/man-pages/man2/connect.2.html
// If the connection or binding succeeds, zero is returned. On error, -1 is returned
bool TCPSocket::connectSocket() {
    StringUtils::printInfo("Tentando conectar ao servidor");
    return (connect(_serverFD, (struct sockaddr *) &_serverAddress, sizeof(_serverAddress)) == 0);
}

// https://man7.org/linux/man-pages/man2/send.2.html
// On success, these calls return the number of bytes sent. On error, -1 is returned
int TCPSocket::sendMessage(char* message) {
    size_t numberOfBytes = send(_serverFD, message, strlen(message), 0);
    //StringUtils::printInfo(to_string(numberOfBytes) + " bytes enviados para o socket " + to_string(_serverFD));
    return numberOfBytes;
}
int TCPSocket::sendMessage(const char* message) {
    size_t numberOfBytes = send(_serverFD, message, strlen(message), 0);
    //StringUtils::printInfo(to_string(numberOfBytes) + " bytes enviados para o socket " + to_string(_serverFD));
    return numberOfBytes;
}
int TCPSocket::sendMessage(int clientFD, char* message) {
    size_t numberOfBytes = send(clientFD, message, strlen(message), 0);
    //StringUtils::printInfo(to_string(numberOfBytes) + " bytes enviados para o socket " + to_string(clientFD));
    return numberOfBytes;
}
int TCPSocket::sendMessage(int clientFD, const char* message) {
    size_t numberOfBytes = send(clientFD, message, strlen(message), 0);
    //StringUtils::printInfo(to_string(numberOfBytes) + " bytes enviados para o socket " + to_string(clientFD));
    return numberOfBytes;
}

int TCPSocket::receive(char* buffer, int maxMessageSize) {
    size_t numberOfBytes = recv(_serverFD, buffer, maxMessageSize, 0);
    //StringUtils::printInfo(to_string(numberOfBytes) + " bytes recebidos do o socket " + to_string(_serverFD));
    //StringUtils::printDanger(buffer);
    return numberOfBytes;
}
int TCPSocket::receive(int clientFD, char* buffer, int maxMessageSize) {
    size_t numberOfBytes = recv(clientFD, buffer, maxMessageSize, 0);
    //StringUtils::printInfo(to_string(numberOfBytes) + " bytes recebidos do o socket " + to_string(clientFD));
    //StringUtils::printDanger(buffer);
    return numberOfBytes;
}

// https://man7.org/linux/man-pages/man2/close.2.html
// returns zero on success.  On error, -1 is returned
bool TCPSocket::closeSocket() {
    return (close(_serverFD) == 0);
} 
bool TCPSocket::closeSocket(int clientFD) {
    return (close(clientFD) == 0);
}

void TCPSocket::printSocketInfo() {
    char ip[INET_ADDRSTRLEN];
    struct in_addr ipAddr = (&_serverAddress)->sin_addr;
    inet_ntop(AF_INET, &ipAddr, ip, INET_ADDRSTRLEN);
    std::string ip_str(ip);
    std::string info = "Servidor rodando em " + ip_str + ":" + to_string(_port);
    StringUtils::printSuccess(info);
}
