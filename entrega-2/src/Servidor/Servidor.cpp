#include "Servidor.hpp"


// https://pubs.opengroup.org/onlinepubs/7908799/xns/syssocket.h.html
// https://www.cs.dartmouth.edu/~campbell/cs60/socketprogramming.html


Servidor::Servidor(char* port) {
    _serverSocket = new TCPSocket(NULL, port);
    if(!_serverSocket->bindServer()) {
        StringUtils::printDanger("Erro ao realizar o bind do socket do servidor");
        exit(1);
    }
    if(!_serverSocket->startListen(MAX_CLIENTS)) {
        StringUtils::printDanger("Erro ao comecar o listen do servidor");
        exit(2);
    }
    sem_init(&_semaphorCurrentFD, 0, 1);
    sem_init(&_semaphorNotifications, 0, 0);
    sem_init(&_semaphorPerfisInclusion, 0, 1);
    _GLOBAL_NOTIFICACAO_ID = 0;
    _saveFileName = "save.csv";
    fillFromFile();

    pthread_t keyboardThead;
    pthread_create(&keyboardThead, NULL, &Servidor::ProcessKeyboardInputStatic, this);
    _isPrimary = true;
}

Servidor::Servidor(char* port, char* primaryIp, char* primaryPort) {
    _serverSocket = new TCPSocket(NULL, port);
    if(!_serverSocket->bindServer()) {
        StringUtils::printDanger("Erro ao realizar o bind do socket do servidor");
        exit(1);
    }
    if(!_serverSocket->startListen(MAX_CLIENTS)) {
        StringUtils::printDanger("Erro ao comecar o listen do servidor");
        exit(2);
    }
    sem_init(&_semaphorCurrentFD, 0, 1);
    sem_init(&_semaphorNotifications, 0, 0);
    sem_init(&_semaphorPerfisInclusion, 0, 1);
    _GLOBAL_NOTIFICACAO_ID = 0;
    _saveFileName = "save.csv";
    fillFromFile();

    pthread_t keyboardThread;
    pthread_create(&keyboardThread, NULL, &Servidor::ProcessKeyboardInputStatic, this);

    connectToPrimary(primaryIp, primaryPort);
    
    _isPrimary = false;
    _minPoolSize = 0;
    _triesToConnectToPrimary = 0;
}

bool Servidor::connectToPrimary(char* primaryIp, char* primaryPort) {
    _primaryServerSocket = new TCPSocket(primaryIp, primaryPort);

    if(!_primaryServerSocket->connectSocket()) {
        StringUtils::printDanger("Problema ao conectar ao servidor");
        return false;
    }
    
    std::string sendPayload(_serverSocket->getSocketIp());
    sendPayload.append(":");
    sendPayload.append(_serverSocket->getSocketPort());
    Pacote* enviado = new Pacote(Tipo::SERVIDOR, time(NULL), Comando::CONNECT, to_string(getpid()), sendPayload);
    _primaryServerSocket->sendMessage(enviado->serializeAsString().c_str());
    char recvLine[MAX_MSG];
    memset(recvLine, 0, sizeof(recvLine));
    _primaryServerSocket->receive(recvLine, MAX_MSG);

    Pacote* recebido = new Pacote(recvLine);
    if(recebido->getStatus() != Status::OK) {
        StringUtils::printDanger(recebido->getPayload());
        _primaryServerSocket->closeSocket();
        return false;
       
    }
    StringUtils::printSuccess(recebido->getPayload());

    return true;
}

void* Servidor::ProcessKeyboardInputStatic(void* context) {
    ((Servidor*)context)->ProcessKeyboardInput();
    pthread_exit(NULL);
}

void Servidor::ProcessKeyboardInput() {
    StringUtils::printInfo("Thread criada para lidar com comandos no servidor!");

    char line[MAX_MSG];
    while(fgets(line, MAX_MSG, stdin) != NULL) {
        StringUtils::removeNewLineAtEnd(line);
        std::string lineString(line);
        Comando comando = Pacote::getComandoFromLine(lineString + " ");
        Pacote p;
        switch (comando)
        {
            case Comando::POOL:
                printPool();
                break;
            case Comando::PERFIS:
                printPerfis();
                break;
            default:
                break;
        }
    }
}

void* Servidor::handleNotificationsStatic(void* context) {
    ((Servidor*)context)->handleNotifications();
    //StringUtils::printDanger("N??o era para chegar aqui");
    pthread_exit(NULL);
}

void Servidor::handleNotifications() {
    while(true) {
        sem_wait(&_semaphorNotifications);
        StringUtils::printInfo("Enviando notificacoes aos usuarios conectados");
        for(int i=0; i<_perfis.size(); i++) {
            sendNotificacoes(&_perfis[i]);
        }
    }
}

void Servidor::start() {
    StringUtils::printSuccess("Servidor iniciado");

    pthread_create(&_notificationHandler, NULL, &Servidor::handleNotificationsStatic, this);

    if(!_isPrimary) {
        servidorPrimarioHandler();
    }
    else {
        while(true) {
            sem_wait(&_semaphorCurrentFD); // Espera a thread que vai lidar com o cliente pegar o clientFD
            StringUtils::printInfo("Esperando algum cliente/servidor conectar... ");
            _currentFD = _serverSocket->acceptConnection();
            if(_currentFD == -1) {
                StringUtils::printDanger("Houve um problema ao conectar com o cliente");
                sem_post(&_semaphorCurrentFD);
                continue;
            }

            char buffer[MAX_MSG];
            memset(buffer, 0, sizeof(buffer));
            _serverSocket->receive(_currentFD, buffer, MAX_MSG);
            Pacote* recebido = new Pacote(buffer);
            if(recebido->getTipo() == Tipo::SERVIDOR) {
                StringUtils::printInfo(recebido->serializeAsString());
                Pacote send;
                sendPacoteToAllServidoresBackup(*recebido);
                send = handleServerConnect(recebido->getUsuario(), recebido->getPayload(), _currentFD);
                _serverSocket->sendMessage(_currentFD, send.serializeAsCharPointer());
                sem_post(&_semaphorCurrentFD);
            }
            else if(recebido->getTipo() == Tipo::CLIENTE){
                Pacote send;

                send = handleClienteConnect(recebido->getUsuario(), _currentFD);
                recebido->setPayload(to_string(_currentFD));
                sendPacoteToAllServidoresBackup(*recebido);
                _serverSocket->sendMessage(_currentFD, send.serializeAsCharPointer());

                pthread_t clientHandlerThread;
                if (pthread_create(&clientHandlerThread, NULL, &Servidor::handleClientStatic, this) != 0) { // Static func of class, this is necessary to keep the context of the class
                    StringUtils::printDanger("Erro ao criar a Thread para lidar com o cliente.");
                    sem_post(&_semaphorCurrentFD);
                }
                else 
                    StringUtils::printInfo("Cliente conectado...");
            }
            else {
                sem_post(&_semaphorCurrentFD);
            }
        }
        _serverSocket->closeSocket();
    }
}

void Servidor::servidorPrimarioHandler() {
    char rcvLine[MAX_MSG];
    memset(rcvLine, 0, sizeof(rcvLine));
    int bytesRead;
    while((bytesRead = _primaryServerSocket->receive(rcvLine, MAX_MSG)) != -1 ) {
        if(bytesRead == 0) {
            // Start election
            _poolSize = _pool.size();
            StringUtils::printWithPrefix(to_string(_poolSize), "Tamanho da Pool: ", Color::NONE);
            resetPool();
            resetClientSockets();
            _minPoolSize = 0;
            if(election()) {
                restartAsPrimary();
            }
            else {
                _triesToConnectToPrimary = 0;
                usleep(TIME_TO_RETRY);
                while(!connectToPrimary(_primaryServerSocket->getSocketIp(), _primaryServerSocket->getSocketPort())) {
                    usleep(TIME_TO_RETRY); // microssegundos = milissegundos*1000
                    _triesToConnectToPrimary++;
                    if(_triesToConnectToPrimary == MAX_RETRIES) {
                        _minPoolSize++;
                        if(election()) {
                            restartAsPrimary();
                        }
                        _triesToConnectToPrimary = 0;
                    }
                }
            }
        }
        std::vector<Pacote> pacotes = Pacote::getMultiplosPacotes(rcvLine);
        Pacote send;
        for(std::vector<Pacote>::iterator pacote = pacotes.begin(); pacote != pacotes.end(); pacote++) {
            if(pacote->getStatus() == Status::OK){
                StringUtils::printBold(pacote->serializeAsString());
                switch (pacote->getComando())
                {
                    case Comando::CONNECT:
                        if(pacote->getTipo() == Tipo::CLIENTE) {
                            send = handleClienteConnect(pacote->getUsuario(), atoi(pacote->getPayload().c_str()));
                            StringUtils::printWithRandomPrefixColor(pacote->getPayload(), pacote->getUsuario() + ":");
                        }
                        else if(pacote->getTipo() == Tipo::SERVIDOR) {
                            send = handleServerConnect(pacote->getUsuario(), pacote->getPayload(), 0);
                        }
                        break;
                    case Comando::SEND:
                        send = handleSend(pacote->getUsuario(), pacote->getTimestamp(), pacote->getPayload(), pacote->getPayload().size());
                        sem_post(&_semaphorNotifications);
                        break;
                    case Comando::FOLLOW:
                        send = handleFollow(pacote->getPayload(), pacote->getUsuario());
                        break;
                    case Comando::DISCONNECT:
                        handleDisconnect(pacote->getUsuario(), atoi(pacote->getPayload().c_str()));
                        send.setStatus(Status::OK);
                        break;
                    case Comando::GETNOTIFICATIONS:
                        send.setStatus(Status::OK);
                        sem_post(&_semaphorNotifications); 
                        break;
                    default:
                        if(pacote->getPayload().size() != 0);
                            StringUtils::printSuccess(pacote->getPayload());
                        break;
                }
                _primaryServerSocket->sendMessage(send.serializeAsCharPointer());
            } 
            else {
                if(pacote->getPayload().size() != 0);
                    StringUtils::printDanger(pacote->getPayload());
            }

        }     
        memset(rcvLine, 0, sizeof(rcvLine));
    }
    _primaryServerSocket->closeSocket();
    StringUtils::printDanger("O servidor encerrou a conex??o");
    exit(4);   
}

void Servidor::restartAsPrimary() {
    //StringUtils::printInfo("Tentando reiniciar como prim??rio");
    usleep(TIME_TO_RESTART);
    _serverSocket->closeSocket();
    _primaryServerSocket->closeSocket();
    _serverSocket = new TCPSocket(_primaryServerSocket->getSocketIp(), _primaryServerSocket->getSocketPort());
    _isPrimary = true;
    while(!_serverSocket->bindServer()) {
        StringUtils::printDanger("Erro ao realizar o bind do socket do servidor");
        sleep(1);
    }
    if(!_serverSocket->startListen(MAX_CLIENTS)) {
        StringUtils::printDanger("Erro ao comecar o listen do servidor");
        exit(2);
    }
    StringUtils::printInfo("Reiniciado como prim??rio");
    start();
}

bool Servidor::election() {
    return (_poolSize == _minPoolSize);
}

void Servidor::resetPool() {
    _pool.clear();
}

void Servidor::resetClientSockets() {
    for(std::vector<Perfil>::iterator perfil = _perfis.begin(); perfil != _perfis.end(); perfil++) {
        perfil->_socketDescriptors.clear();
    }
}

Pacote Servidor::handleServerConnect(std::string pid, std::string payload, int FD) {
    Pacote send(Tipo::DATA, Status::OK, "Servidor conectado com sucesso!");
    std::string ip = payload.substr(0, payload.find_first_of(":"));
    std::string port = payload.substr(payload.find_first_of(":") + 1);

    int foundIndex = -1;

    for(int i=0; i<_pool.size(); i++) {
        if(_pool[i].Ip == ip && _pool[i].Port == port) {
            foundIndex = i;
            break;
        }
    }
    if(foundIndex == -1) {
        ServerPerfil servidor;
        servidor.PID = (pid_t)atoi(pid.c_str());
        
        servidor.Ip = ip;
        servidor.Port = port;
        servidor.FD = FD;
        servidor.isAlive = true;
        _pool.push_back(servidor);
    }
    else {
        _pool[foundIndex].FD = FD;
    }

    return send;
}

void Servidor::sendPacoteToAllServidoresBackup(Pacote pacote) {
    for(std::vector<ServerPerfil>::iterator server = _pool.begin(); server != _pool.end(); server++ ) {
        if(server->isAlive) {
            _serverSocket->sendMessage(server->FD, pacote.serializeAsCharPointer());
            char rcvLine[MAX_MSG];
            memset(rcvLine, 0, sizeof(rcvLine));
            int bytesRead;
            bytesRead = _serverSocket->receive(server->FD, rcvLine, MAX_MSG);
            if(bytesRead == -1 || bytesRead == 0) {
                server->isAlive = false;
                continue;
            }
            Pacote* rcv = new Pacote(rcvLine);
            if(rcv->getStatus() == Status::OK)
                continue;
        }
    }
}

void* Servidor::handleClientStatic(void* context) {
    ((Servidor*)context)->handleClient();
    pthread_exit(NULL);
}

void Servidor::handleClient() {
    int n;
    int clientFD = _currentFD;
    char buffer[MAX_MSG];

    sem_post(&_semaphorCurrentFD); // Libera a pr??xima conex??o depois de pegar o clientFD

    Pacote send;
    StringUtils::printWarning("Thread criada para lidar com as requisicoes do cliente");

    //updateClientePool(_currentFD);

    memset(buffer, 0, sizeof(buffer));
    while( (n = _serverSocket->receive(clientFD, buffer, MAX_MSG)) > 0 ) {
        //StringUtils::printBold(buffer);
        std::vector<Pacote> pacotes = Pacote::getMultiplosPacotes(buffer);
        for(std::vector<Pacote>::iterator recebido = pacotes.begin(); recebido != pacotes.end(); recebido++) {
            StringUtils::printBold(recebido->serializeAsString());
            switch (recebido->getComando())
            {
                case Comando::DISCONNECT:
                    handleDisconnect(recebido->getUsuario(), clientFD);
                    recebido->setPayload(to_string(clientFD));
                    sendPacoteToAllServidoresBackup(*recebido);
                    StringUtils::printWarning("Desconectando " + recebido->getUsuario() + " no socket " + to_string(clientFD) + ". Motivo: Usuario deslogado");
                    _serverSocket->closeSocket(clientFD);
                    pthread_exit(NULL);
                    break;
                case Comando::SEND:
                    send = handleSend(recebido->getUsuario(), recebido->getTimestamp(), recebido->getPayload(), recebido->getTamanhoPayload());
                    if(send.getStatus() == Status::OK)
                        sendPacoteToAllServidoresBackup(*recebido);
                    sem_post(&_semaphorNotifications);
                    break;
                case Comando::FOLLOW:
                    send = handleFollow(recebido->getPayload(), recebido->getUsuario());
                    if(send.getStatus() == Status::OK)
                        sendPacoteToAllServidoresBackup(*recebido);
                    break;
                case Comando::GETNOTIFICATIONS:
                    sem_post(&_semaphorNotifications);
                    sendPacoteToAllServidoresBackup(*recebido);
                    break;
                case Comando::TESTE:
                    printPerfis();
                    break;
                default:
                    break;
            }
            
            if(recebido->getComando() != Comando::GETNOTIFICATIONS) {
                if(_serverSocket->sendMessage(clientFD, send.serializeAsString().c_str()) == -1)
                    StringUtils::printDanger("Erro ao enviar a mensagem de volta");
            }
        }
        
        memset(buffer, 0, sizeof(buffer));
        StringUtils::printInfo("Esperando mensagem do cliente...");
    }

    _serverSocket->closeSocket(clientFD);

    if (n < 0)
        StringUtils::printDanger("Houve um problema ao ler a mensagem do cliente");
    else if (n == 0)
        StringUtils::printWarning("Cliente se desconectou do servidor");
}

Pacote Servidor::handleClienteConnect(string usuario, int socketDescriptor) {
    bool conectado = false;
    int index;
    Pacote send(Tipo::DATA, Status::OK, "Usuario conectado com sucesso!");
    StringUtils::printInfo("Lidando com a conexao do usuario " + usuario + " no socket " + to_string(socketDescriptor));
    
    sem_wait(&_semaphorPerfisInclusion);
    for(int i=0; i<_perfis.size(); i++) {
        if(_perfis[i]._usuario == usuario) {
            conectado = true;
            index = i;
            break;
        }
    }
    if(conectado) {
        // Caso esteja com duas conex???os da ruim
        sem_wait(&_perfis[index]._semaphorePerfil);
        if(_perfis[index]._socketDescriptors.size() == 2) {
            if(_perfis[index]._socketDescriptors[0] != socketDescriptor || _perfis[index]._socketDescriptors[1] == socketDescriptor) {
                string message = "Numero maximo de conexoes para o usuario excedido";
                send.setStatus(Status::ERROR);
                send.setPayload(message);
                StringUtils::printWarning("Desconectando " + usuario + " no socket " +  to_string(socketDescriptor) + ". Motivo: " + message);
            }
        }
        else if(_perfis[index]._socketDescriptors.size() == 1 && _perfis[index]._socketDescriptors[0] != socketDescriptor) {
            _perfis[index]._socketDescriptors.push_back(socketDescriptor);
        }
        else {
            _perfis[index]._socketDescriptors.push_back(socketDescriptor);
        }
        sem_post(&_perfis[index]._semaphorePerfil);
    }
    else {
        Perfil novoPerfil(usuario, socketDescriptor);
        _perfis.push_back(novoPerfil);
    }
    sem_post(&_semaphorPerfisInclusion);
    // fecha socket e termina a thread caso usuario tenha sido "deslogado"
    if(_isPrimary && send.getStatus() == Status::ERROR) {
        _serverSocket->sendMessage(socketDescriptor, send.serializeAsCharPointer());
        //StringUtils::printDanger("Fechando socket " + socketDescriptor);
        _serverSocket->closeSocket(socketDescriptor);
        pthread_exit(NULL);
    }

    return send;
}

void Servidor::handleDisconnect(string usuario, int socketDescriptor) {
    for(vector<Perfil>::iterator perfil = _perfis.begin(); perfil != _perfis.end(); perfil++) {
        sem_wait(&perfil->_semaphorePerfil);
        if(perfil->_usuario == usuario) {
            for(vector<int>::iterator socket = perfil->_socketDescriptors.begin(); socket != perfil->_socketDescriptors.end(); socket++) {
                if(*socket == socketDescriptor) {
                    perfil->_socketDescriptors.erase(socket);
                    break;
                }
            }
            sem_post(&perfil->_semaphorePerfil);
            break;
        }
        sem_post(&perfil->_semaphorePerfil);
    }
}

Pacote Servidor::handleSend(std::string usuario, time_t timestamp, std::string payload, int tamanhoPayload) {
    Pacote p;
    p.setStatus(Status::OK);
    p.setComando(Comando::NO);
    p.setPayload("Mensagem enviada com sucesso!");

    Notificacao notificacao;
    notificacao._id = _GLOBAL_NOTIFICACAO_ID++;
    notificacao._timestamp = timestamp;
    notificacao._mensagem = payload;
    notificacao._tamanho = tamanhoPayload;    

    std::vector<string> seguidores;
    for(std::vector<Perfil>::iterator perfil = _perfis.begin(); perfil != _perfis.end(); perfil++) {
        sem_wait(&perfil->_semaphorePerfil);
        if(perfil->_usuario == usuario) {
            notificacao._quantidadeSeguidoresAReceber = perfil->_seguidores.size();
            notificacao.printNotificacao();
            if(notificacao._quantidadeSeguidoresAReceber != 0) {
                perfil->_notificacoesRecebidas.push_back(notificacao);
                seguidores = perfil->_seguidores;
            }
        }
        sem_post(&perfil->_semaphorePerfil);
    }

    for(std::vector<Perfil>::iterator perfil = _perfis.begin(); perfil != _perfis.end(); perfil++) {
        sem_wait(&perfil->_semaphorePerfil);
        for(std::vector<std::string>::iterator seguidor = seguidores.begin(); seguidor != seguidores.end(); seguidor++) {
            if(perfil->_usuario == *seguidor) {
                perfil->_notificacoesPendentes.push_back(std::make_pair(usuario, notificacao._id));
            }
        }
        sem_post(&perfil->_semaphorePerfil);
    }
    
    return p;
}

Pacote Servidor::handleFollow(std::string usuarioSeguido, std::string usuarioSeguidor) {
    bool existeSeguido = false;
    bool jaSegue = false;
    Pacote send(Tipo::DATA, Status::OK, "Seguindo usuario " + usuarioSeguido);
    // Procura se o usu??rio est?? logado j??
    for(std::vector<Perfil>::iterator perfil = _perfis.begin(); perfil != _perfis.end(); perfil++) { 
        if(perfil->_usuario == usuarioSeguido) {    
            sem_wait(&perfil->_semaphorePerfil);
            existeSeguido = true;
            for(std::vector<std::string>::iterator seguidor = perfil->_seguidores.begin(); seguidor != perfil->_seguidores.end(); seguidor++) {
                if(*seguidor == usuarioSeguidor) {
                    jaSegue = true;
                    send.setStatus(Status::ERROR);
                    send.setPayload("Voce ja segue este usuario!");
                    break;
                }
            }
            if(!jaSegue) {
                StringUtils::printInfo("Usuario " + usuarioSeguidor + " comecou a seguir " + usuarioSeguido);
                perfil->_seguidores.push_back(usuarioSeguidor);
            }
            sem_post(&perfil->_semaphorePerfil);
            break;
        }
    }
    if(!existeSeguido) {
        send.setStatus(Status::ERROR);
        send.setPayload("O perfil " + usuarioSeguido + " nao existe logo nao e possivel segui-lo.");
    }
    return send;
}

Pacote Servidor::sendNotificacao(std::string from, int idNotificacao) {
    Pacote p;
    p.setStatus(Status::OK);
    p.setComando(Comando::NO);
    p.setPayload("Mensagem enviada com sucesso!");
    for(std::vector<Perfil>::iterator perfil = _perfis.begin(); perfil != _perfis.end(); perfil++) {
        //StringUtils::printBold("Achou perfil from");
        if(perfil->_usuario == from) {
            sem_wait(&perfil->_semaphorePerfil);
            for(std::vector<Notificacao>::iterator notificacao = perfil->_notificacoesRecebidas.begin(); notificacao != perfil->_notificacoesRecebidas.end(); notificacao++) {
                if(notificacao->_id == idNotificacao) {
                    //StringUtils::printBold("Achou notificacao");
                    Pacote* pacote = new Pacote(Tipo::DATA, notificacao->_timestamp, Comando::NOTIFICATION, from, notificacao->_mensagem);
                    for(std::vector<Perfil>::iterator p = _perfis.begin(); p != _perfis.end(); p++) {
                        if(p->_usuario == perfil->_usuario)
                            continue;
                       // StringUtils::printInfo("Adiquirindo lock do perfil" + p->_usuario);
                        sem_wait(&p->_semaphorePerfil);
                        //StringUtils::printInfo("Adquirido lock do perfil" + p->_usuario);
                        for(std::vector<std::pair<std::string,int>>::iterator par = p->_notificacoesPendentes.begin(); par != p->_notificacoesPendentes.end(); par++) {
                            if(par->second == idNotificacao) {
                                //StringUtils::printBold("Achou Notificacao no perfil to");
                                bool sent = false;
                                for(int i=0; i<p->_socketDescriptors.size(); i++) {
                                    //StringUtils::printDanger("Enviando notificacao");
                                    if(_isPrimary) 
                                        _serverSocket->sendMessage(p->_socketDescriptors[i], pacote->serializeAsString().c_str());
                                    sent = true;
                                }
                                if(sent) {
                                    p->_notificacoesPendentes.erase(par);
                                    notificacao->_quantidadeSeguidoresAReceber--;
                                    break;
                                }
                            }
                        }
                        //StringUtils::printInfo("Liberando lock do perfil" + p->_usuario);
                        sem_post(&p->_semaphorePerfil);
                        //StringUtils::printInfo("Liberado lock do perfil" + p->_usuario);
                    }
                    if(notificacao->_quantidadeSeguidoresAReceber == 0) {
                        perfil->_notificacoesRecebidas.erase(notificacao);
                        break;
                    }
                }
            }
            sem_post(&perfil->_semaphorePerfil);
            break;
        }
    }
    return p;
}


void Servidor::sendNotificacoes(Perfil* to) {
    std::vector<std::pair<std::string, int>> notificacoes;
    //StringUtils::printDanger("Tentando adicionar notificacao DO USER: " + to->_usuario);
    sem_wait(&to->_semaphorePerfil);
    //StringUtils::printDanger("Adquirido lock do perfil " + to->_usuario);
    for(int i=0; i < to->_notificacoesPendentes.size(); i++ ) {
        notificacoes.push_back(std::make_pair(to->_notificacoesPendentes[i].first, to->_notificacoesPendentes[i].second));
    }
    //StringUtils::printDanger("Liberando lock do perfil " + to->_usuario);
    sem_post(&to->_semaphorePerfil);
    //StringUtils::printDanger("Adicionadas notificacoes");

    for(int i=0; i < notificacoes.size(); i++ ) {
        sendNotificacao(notificacoes[i].first, notificacoes[i].second);
    }
}




void Servidor::info() {
    _serverSocket->printSocketInfo();
}

bool Servidor::checkStartupParameters(int argc, char** argv) {
    int port;
    if(argc < 2) {
        StringUtils::printDanger("Quantidade de parametros passados eh insuficiente");
        return false;
    }

    if((!strcmp(argv[1], "--help") || !strcmp(argv[1], "-h"))) 
        return false;

    string s(argv[1]);
    if(s.find_first_not_of("0123456789") != string::npos) {
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
    cout << endl;
    StringUtils::printBold("USAGE");
    puts("./app_servidor [-h | --help] <PORTA>");
    puts("Onde <PORTA> esta no intervalo [1, 65535]");
    cout << endl;

    StringUtils::printBold("EXAMPLES");
    puts("./app_servidor 8080");
    puts("./app_servidor 8888");
    puts("./app_servidor -h");
    puts("./app_servidor --help");
    cout << endl;

    StringUtils::printBold("DESCRIPTION");
    cout << "Programa para criacao de um servidor para atender os requisitos da parte 1" << endl << "do trabalho pratico da cadeira de Sistemas Operacionais II" << endl;
}

void Servidor::printPerfis() {
    for(Perfil p : _perfis) {
        p.printPerfil();
    }
}

void Servidor::gracefullShutDown() {
    
    if(_isPrimary) {
        saveFile();
    }
    sem_destroy(&_semaphorCurrentFD);
    sem_destroy(&_semaphorNotifications);
    if(_serverSocket->closeSocket())
        StringUtils::printSuccess("Servidor desligado com sucesso!");
    
    exit(0);
}

void Servidor::saveFile() {
    std::ofstream file(_saveFileName, std::ofstream::out);

    for(std::vector<Perfil>::iterator perfil = _perfis.begin(); perfil != _perfis.end(); perfil++) {
        std::string line = "";
        line = perfil->_usuario + ";";
        for(std::vector<string>::iterator seguidor = perfil->_seguidores.begin(); seguidor != perfil->_seguidores.end(); seguidor++) {
            line += *seguidor + ";";
        }
        line += "\n";
        file << line;
    }

    file.close();
}

void Servidor::fillFromFile() {
    std::ifstream file(_saveFileName ,std::ifstream::in);

    while(file.good()) {
        Perfil novoPerfil;
        std::string line = "";
        file >> line;
        if(line.size() > 1) {
            std::string usuario = line.substr(0, line.find_first_of(";"));
            novoPerfil._usuario = usuario;
            line = line.substr(line.find_first_of(";")+1);
            std::size_t pontoVirgula;
            while((pontoVirgula = line.find_first_of(";")) != std::string::npos) {
                std::string seguidor = line.substr(0, pontoVirgula);
                novoPerfil._seguidores.push_back(seguidor);
                line = line.substr(pontoVirgula+1);
            }
            _perfis.push_back(novoPerfil);
        }
    }
    file.close();
}

void Servidor::printPool() {
    if(_pool.size() != 0) {
        for(int i=0; i<_pool.size(); i++) {
            StringUtils::printBold("Servidor " + to_string(i) + ":");
            StringUtils::printWithRandomPrefixColor(to_string(_pool[i].PID), "PID:");
            StringUtils::printWithRandomPrefixColor(_pool[i].Ip, "IP:");
            StringUtils::printWithRandomPrefixColor(_pool[i].Port, "Port:");
            StringUtils::printWithRandomPrefixColor(to_string(_pool[i].FD), "FD:");
            StringUtils::printWithRandomPrefixColor((_pool[i].isAlive ? "Sim": "Nao"), "IsAlive:");
        }
    } 
    else {
        StringUtils::printInfo("Servidor ainda n??o possui servidores backup conectados");
    }
}