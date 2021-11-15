#include "TCPSocket.hpp"

TCPSocket::TCPSocket() {
    //StringUtils::printInfo("Criando socket...1");
    if (!createSocket()) {
        //StringUtils::printDanger("Problema ao criar o socket!");
        exit(2);
    }
    int t = 1;
    setsockopt(_serverFD,SOL_SOCKET,SO_REUSEADDR,&t,sizeof(int));
    

    _ip = (char*) malloc(sizeof(char)*(strlen(DEFAULT_IP_CHAR_POINTER) + 1));
    strcpy(_ip, DEFAULT_IP_CHAR_POINTER);

    _port = (char*) malloc(sizeof(char)*strlen(DEFAULT_PORT_CHAR_POINTER));
    strcpy(_port, DEFAULT_PORT_CHAR_POINTER);

    memset(&_serverAddress, 0, sizeof(_serverAddress));
    _serverAddress.sin_family = AF_INET;
    _serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    _serverAddress.sin_port = htons(DEFAULT_PORT);

    //StringUtils::printSuccess("Socket criada com sucesso");
}

TCPSocket::TCPSocket(char* serverIp, char* serverPort) {
    // https://man7.org/linux/man-pages/man2/socket.2.html
    // On success, a file descriptor for the new socket is returned. On error, -1 is returned
    
    //StringUtils::printInfo("Criando socket...2");
    if (!createSocket()) {
        //StringUtils::printDanger("Problema ao criar o socket!");
        exit(2);
    }
    int t = 1;
    setsockopt(_serverFD,SOL_SOCKET,SO_REUSEADDR,&t,sizeof(int));

    if(serverIp != NULL) {
        _ip = (char*) malloc(sizeof(char)*(strlen(serverIp) + 1));
        strcpy(_ip, serverIp);
    } else {
        _ip = (char*) malloc(sizeof(char)*(strlen(DEFAULT_IP_CHAR_POINTER) + 1));
        strcpy(_ip, DEFAULT_IP_CHAR_POINTER);
    }
       
    _port = (char*) malloc(sizeof(char)*(strlen(serverPort) + 1));
    strcpy(_port, serverPort);

    memset(&_serverAddress, 0, sizeof(_serverAddress));
    _serverAddress.sin_family = AF_INET;
    _serverAddress.sin_addr.s_addr = (serverIp != NULL) ? htonl(inet_addr(serverIp)) : htonl(INADDR_ANY);
    _serverAddress.sin_port = htons(atoi(_port));
    //StringUtils::printSuccess("Socket criada com sucesso");    
}


TCPSocket::TCPSocket(const char* serverIp, const char* serverPort) {
    // https://man7.org/linux/man-pages/man2/socket.2.html
    // On success, a file descriptor for the new socket is returned. On error, -1 is returned
    
    //StringUtils::printInfo("Criando socket...3");
    if (!createSocket()) {
        //StringUtils::printDanger("Problema ao criar o socket!");
        exit(2);
    }
    int t = 1;
    setsockopt(_serverFD,SOL_SOCKET,SO_REUSEADDR,&t,sizeof(int));
    
    if(serverIp != NULL) {
        _ip = (char*) malloc(sizeof(char)*(strlen(serverIp) + 1));
        strcpy(_ip, serverIp);
    } else {
        _ip = (char*) malloc(sizeof(char)*(strlen(DEFAULT_IP_CHAR_POINTER) + 1));
        strcpy(_ip, DEFAULT_IP_CHAR_POINTER);
    }
       
    _port = (char*) malloc(sizeof(char)*(strlen(serverPort) + 1));
    strcpy(_port, serverPort);


    memset(&_serverAddress, 0, sizeof(_serverAddress));
    _serverAddress.sin_family = AF_INET;
    _serverAddress.sin_addr.s_addr = (serverIp != NULL) ? htonl(inet_addr(serverIp)) : htonl(INADDR_ANY);
    _serverAddress.sin_port = htons(atoi(_port));
    //StringUtils::printSuccess("Socket criada com sucesso");
}

bool TCPSocket::createSocket() {
    return ((_serverFD = socket(AF_INET, SOCK_STREAM, 0)) > 0);
}

bool TCPSocket::bindServer() {
    // https://man7.org/linux/man-pages/man2/bind.2.html
    // On success, zero is returned.  On error, -1 is returned

    #ifdef __APPLE__
        return (::bind(_serverFD, (struct sockaddr *) &_serverAddress, sizeof(_serverAddress)) == 0);
    #else
        return (bind(_serverFD, (struct sockaddr *) &_serverAddress, sizeof(_serverAddress)) == 0);
    #endif
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
    string ip(_ip);
    string port(_port);
    //StringUtils::printInfo("Tentando conectar ao servidor " + ip + ":" + port);
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
int TCPSocket::sendMessage(int FD, char* message) {
    size_t numberOfBytes = send(FD, message, strlen(message), 0);
    //StringUtils::printInfo(to_string(numberOfBytes) + " bytes enviados para o socket " + to_string(clientFD));
    return numberOfBytes;
}
int TCPSocket::sendMessage(int FD, const char* message) {
    size_t numberOfBytes = send(FD, message, strlen(message), 0);
    //StringUtils::printInfo(to_string(numberOfBytes) + " bytes enviados para o socket " + to_string(clientFD));
    return numberOfBytes;
}

int TCPSocket::receive(char* buffer, int maxMessageSize) {
    size_t numberOfBytes = recv(_serverFD, buffer, maxMessageSize, 0);
    //StringUtils::printInfo(to_string(numberOfBytes) + " bytes recebidos do o socket " + to_string(_serverFD));
    //StringUtils::printDanger(buffer);
    return numberOfBytes;
}
int TCPSocket::receive(int FD, char* buffer, int maxMessageSize) {
    size_t numberOfBytes = recv(FD, buffer, maxMessageSize, 0);
    //StringUtils::printInfo(to_string(numberOfBytes) + " bytes recebidos do o socket " + to_string(clientFD));
    //StringUtils::printDanger(buffer);
    return numberOfBytes;
}

// https://man7.org/linux/man-pages/man2/close.2.html
// returns zero on success.  On error, -1 is returned
bool TCPSocket::closeSocket() {
    return (close(_serverFD) == 0);
} 
bool TCPSocket::closeSocket(int FD) {
    return (close(FD) == 0);
}

char* TCPSocket::getSocketIp() {
    return _ip;
}
char* TCPSocket::getSocketPort() {
    return _port;
}
int TCPSocket::getServerFD() {
    return _serverFD;
}
void TCPSocket::unbindServerSocket() {
    int t = 1;
    setsockopt(_serverFD,SOL_SOCKET,SO_REUSEADDR,&t,sizeof(int));
}

void TCPSocket::printSocketInfo() {
    char ip[INET_ADDRSTRLEN];
    struct in_addr ipAddr = (&_serverAddress)->sin_addr;
    inet_ntop(AF_INET, &ipAddr, ip, INET_ADDRSTRLEN);
    std::string ip_str(ip);
    std::string port_str(_port);
    std::string info = "Servidor rodando em " + ip_str + ":" + port_str;
    StringUtils::printSuccess(info);
}
