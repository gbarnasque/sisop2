#include "FrontEnd.hpp"

FrontEnd::FrontEnd(char* serverIp, char* serverPort, char* user) {
    std::string usuario(user);
    _usuario = usuario;

    connectToServer(serverIp, serverPort);

    sem_init(&_semaphorSendPacote, 0, 1);
}

bool FrontEnd::connectToServer(const char* serverIp, const char* serverPort) {
    
    char recvLine[MAX_MSG];
    _socket = new TCPSocket(serverIp, serverPort);

    if(!_socket->connectSocket()) {
        return false;
    }
    Pacote* enviado = new Pacote(Tipo::CLIENTE, time(NULL), Comando::CONNECT, _usuario);
    _socket->sendMessage(enviado->serializeAsCharPointer());

    memset(recvLine, 0, sizeof(recvLine));
    _socket->receive(recvLine, MAX_MSG);

    Pacote* recebido = new Pacote(recvLine);
    if(recebido->getStatus() != Status::OK) {
        _socket->closeSocket();
        return false;
    }

    enviado->setComando(Comando::GETNOTIFICATIONS);
    _socket->sendMessage(enviado->serializeAsCharPointer());

    return true;
}

void FrontEnd::receiveNotifications() {
    StringUtils::printInfo("[RECEIVENOTIFICATIONS] Thread2 iniciada!");
    
    char rcvLine[MAX_MSG];
    memset(rcvLine, 0, sizeof(rcvLine));
    int bytesRead;
    while((bytesRead = _socket->receive(rcvLine, MAX_MSG)) != -1 ) {
        if(bytesRead == 0) {
            _serverDown = true;
            sem_wait(&_semaphorSendPacote);
            memset(rcvLine, 0, sizeof(rcvLine));
            usleep(TIME_TO_RETRY);
            _socket->closeSocket();
            _triesToConnect = 0;
            while(!connectToServer(_socket->getSocketIp(), _socket->getSocketPort())) {
                usleep(TIME_TO_RETRY); // microssegundos = milissegundos*1000
                _triesToConnect++;
                if(_triesToConnect == MAX_RETRIES) {
                   StringUtils::printDanger("Erro ao tentar reconectar ao servidor após " + to_string(MAX_RETRIES) + " vezes!");
                   exit(5);
                }
            };
            StringUtils::printSuccess("Reconectado no servidor");
            sem_post(&_semaphorSendPacote);
            _serverDown = false;
            sendPacotes();
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
    if(!_serverDown) {
        sendPacote(*p);
    }
    else {
        _pacotes.push_back(*p);
    }
}

void FrontEnd::sendPacotes() {
    for(int i=0; i<_pacotes.size(); i++) {
        _socket->sendMessage(_pacotes[i].serializeAsCharPointer());
    }
    _pacotes.clear();
}