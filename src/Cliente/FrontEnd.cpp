#include "FrontEnd.hpp"

FrontEnd::FrontEnd(char* serverIp, char* serverPort, char* user) {
    std::string usuario(user);
    _usuario = usuario;

    connectToServer(serverIp, serverPort);

    sem_init(&_semaphorSendPacote, 0, 1);
}

void FrontEnd::connectToServer(const char* serverIp, const char* serverPort) {
    char recvLine[MAX_MSG];
    _socket = new TCPSocket(serverIp, serverPort);

    if(!_socket->connectSocket()) {
        StringUtils::printDanger("Problema ao conectar ao servidor");
        exit(3);
    }
    Pacote* enviado = new Pacote(Tipo::CLIENTE, time(NULL), Comando::CONNECT, _usuario);
    _socket->sendMessage(enviado->serializeAsString().c_str());

    memset(recvLine, 0, sizeof(recvLine));
    _socket->receive(recvLine, MAX_MSG);

    Pacote* recebido = new Pacote(recvLine);
    if(recebido->getStatus() == Status::OK) {
        StringUtils::printSuccess(recebido->getPayload());
        enviado->setComando(Comando::GETNOTIFICATIONS);
        _socket->sendMessage(enviado->serializeAsString().c_str());
    }
    else {
        StringUtils::printDanger(recebido->getPayload());
        _socket->closeSocket();
        exit(4);
    }
    
}

void FrontEnd::receiveNotifications() {
    StringUtils::printInfo("[RECEIVENOTIFICATIONS] Thread2 iniciada!");
    
    char rcvLine[MAX_MSG];
    memset(rcvLine, 0, sizeof(rcvLine));
    int bytesRead;
    while((bytesRead = _socket->receive(rcvLine, MAX_MSG)) != -1 ) {
        if(bytesRead == 0) {
            sem_wait(&_semaphorSendPacote);
            memset(rcvLine, 0, sizeof(rcvLine));
            sleep(2);
            _socket->closeSocket();
            connectToServer(_socket->getSocketIp(), _socket->getSocketPort());
            StringUtils::printSuccess("Reconectado no servidor.");
            sem_post(&_semaphorSendPacote);
        }
        std::vector<Pacote> pacotes = Pacote::getMultiplosPacotes(rcvLine);
        
        for(std::vector<Pacote>::iterator pacote = pacotes.begin(); pacote != pacotes.end(); pacote++) {
            if(pacote->getStatus() == Status::OK){
                ServerPerfil servidorBackup;
                std::string payload = pacote->getPayload();
                switch (pacote->getComando())
                {
                    case Comando::NOTIFICATION:
                        StringUtils::printWithRandomPrefixColor(pacote->getPayload(), pacote->getUsuario() + ":");
                        break;
                    /*case Comando::DISCONNECT:
                        StringUtils::printDanger(pacote->getPayload());
                        handleExit();
                        break;
                        */
                    /*
                    case Comando::POOL:
                        servidorBackup.PID = (pid_t)atoi(pacote->getUsuario().c_str());
                        servidorBackup.Ip = payload.substr(0, payload.find_first_of(":"));
                        servidorBackup.Port = payload.substr(payload.find_first_of(":") + 1);
                        servidorBackup.isPrimary = pacote->getIsPrimary();
                        servidorBackup.isAlive = pacote->getIsAlive();                        
                        _servidores.push_back(servidorBackup);
                        */
                        break;
                    default:
                        if(pacote->getPayload().size() != 0);
                            StringUtils::printSuccess(pacote->getPayload());
                        break;
                }
            } 
            else {
                if(pacote->getPayload().size() != 0);
                    StringUtils::printDanger(pacote->getPayload());
            }
        }     
        memset(rcvLine, 0, sizeof(rcvLine));
    }
    _socket->closeSocket();
    StringUtils::printDanger("O servidor encerrou a conexão");
    exit(4);   
}
/*
void FrontEnd::printPool() {
    if(_servidores.size() != 0) {
        for(int i=0; i<_servidores.size(); i++) {
            StringUtils::printBold("Servidor " + to_string(i) + ":");
            StringUtils::printWithRandomPrefixColor(to_string(_servidores[i].PID), "PID:");
            StringUtils::printWithRandomPrefixColor(_servidores[i].Ip, "IP:");
            StringUtils::printWithRandomPrefixColor(_servidores[i].Port, "Port:");
            StringUtils::printWithRandomPrefixColor(to_string(_servidores[i].FD), "FD:");
            StringUtils::printWithRandomPrefixColor(_servidores[i].isPrimary ? "Yes" : "No", "IsPrimary:");
            StringUtils::printWithRandomPrefixColor(_servidores[i].isAlive ? "Yes" : "No", "IsAlive:");
        }
    } 
    else {
        StringUtils::printInfo("Cliente ainda não possui servidores backup listados");
    }
}
*/

void FrontEnd::handleExit() {
    Pacote* p = new Pacote(Tipo::COMMAND, time(NULL), Comando::DISCONNECT, _usuario);
    _socket->sendMessage(p->serializeAsString().c_str());
    _socket->closeSocket();
    exit(0);
}

void FrontEnd::sendPacote(Pacote pacote) {
    sem_wait(&_semaphorSendPacote);
    pacote.setUsuario(_usuario);
    _socket->sendMessage(pacote.serializeAsString().c_str());
    sem_post(&_semaphorSendPacote);
}

void FrontEnd::sendPacote(Comando comando, std::string payload) {
    Pacote* p = new Pacote(Tipo::COMMAND, time(NULL), comando, _usuario, payload);
    sem_wait(&_semaphorSendPacote);
    _socket->sendMessage(p->serializeAsString().c_str());
    sem_post(&_semaphorSendPacote);
}