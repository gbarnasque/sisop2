#include "Servidor.hpp"


// https://pubs.opengroup.org/onlinepubs/7908799/xns/syssocket.h.html
// https://www.cs.dartmouth.edu/~campbell/cs60/socketprogramming.html


Servidor::Servidor(char* port, char* primaryServerPort) {
    _serverSocket = new TCPSocket(NULL, port);
    if(!_serverSocket->bindServer()) {
        StringUtils::printDanger("Erro ao realizar o bind do socket do servidor");
        exit(1);
    }
    if(!_serverSocket->startListen(MAX_CLIENTS)) {
        StringUtils::printDanger("Erro ao comecar o listen do servidor");
        exit(2);
    }
    sem_init(&_semaphorClientFD, 0, 1);
    sem_init(&_semaphorNotifications, 0, 0);
    sem_init(&_semaphorPerfisInclusion, 0, 1);
    _GLOBAL_NOTIFICACAO_ID = 0;
    _saveFileName = "save.csv";
    fillFromFile();

    
    _primaryServerSocket = NULL;
    _isPrimary = true;
    sem_init(&_semaphorPool, 0, 1);
    if(strcmp(primaryServerPort, "0") != 0) {
        _primaryServerSocket = new TCPSocket(DEFAULT_IP, primaryServerPort);
        if(!_primaryServerSocket->connectSocket()) {
            StringUtils::printDanger("Problema ao conectar ao servidor");
            exit(3);
        }
        
        _isPrimary = false;

        pthread_t serverNotificationsHandler;
        pthread_create(&serverNotificationsHandler, NULL, &Servidor::handleServerNotificationsStatic, this);


        Pacote p;
        p.setComando(Comando::CONNECTSERVERTOPOOL);
        p.setUsuario(to_string(getpid()));
        p.setPayload(to_string(_serverSocket->getServerFD()));
        _primaryServerSocket->sendMessage(p.serializeAsCharPointer());

        char recvLine[MAX_MSG];
        memset(recvLine, 0, sizeof(recvLine));
        _primaryServerSocket->receive(recvLine, MAX_MSG);

        Pacote* recebido = new Pacote(recvLine);
        if(recebido->getStatus() == Status::ERROR) {
            StringUtils::printDanger(recebido->getPayload());
            _primaryServerSocket->closeSocket();
            exit(4);
        }
        StringUtils::printSuccess("Conectado no servidor com sucesso!");
    }
}
void* Servidor::handleServerNotificationsStatic(void* context) {
    ((Servidor*)context)->handleServerNotifications();
    pthread_exit(NULL);
}
void Servidor::handleServerNotifications() {
    while(true) {
        char rcvLine[MAX_MSG];
        memset(rcvLine, 0, sizeof(rcvLine));
        while(_serverSocket->receive(rcvLine, MAX_MSG) != -1) {
            std::vector<Pacote> pacotes = Pacote::getMultiplosPacotes(rcvLine);

            for(std::vector<Pacote>::iterator pacote = pacotes.begin(); pacote != pacotes.end(); pacote++) {
                if(pacote->getStatus() == Status::OK) {
                    Pacote send;
                    switch(pacote->getComando()) {
                        case Comando::UPDATEPOOLINSERT:
                            sem_wait(&_semaphorPool);
                            updatePool(pacote->getUsuario(), pacote->getPayload());
                            sem_post(&_semaphorPool);
                            send.setStatus(Status::OK);
                            send.setUsuario(to_string(getpid()));
                            _primaryServerSocket->sendMessage(send.serializeAsCharPointer());
                            break;
                        
                        default:
                            break;
                    }
                }
                else if(pacote->getPayload().size() != 0)
                    StringUtils::printDanger(pacote->getPayload());
            }
        }
    }
}

void* Servidor::handleNotificationsStatic(void* context) {
    ((Servidor*)context)->handleNotifications();
    pthread_exit(NULL);
}

void Servidor::handleNotifications() {
    while(true) {
        sem_wait(&_semaphorNotifications);
        StringUtils::printInfo("Enviando notificacoes aos usuarios conectados");
        //printPerfis();
        for(std::vector<Perfil>::iterator perfil = _perfis.begin(); perfil != _perfis.end(); perfil++) {
            sendNotificacoes(*perfil);
        }
    }
}

void Servidor::start() {
    StringUtils::printSuccess("Servidor iniciado");

    pthread_t notificationHandler;
    pthread_create(&notificationHandler, NULL, &Servidor::handleNotificationsStatic, this);
    if(!_isPrimary){
        pthread_t healthCheckHandler;
        pthread_create(&healthCheckHandler, NULL, &Servidor::handleHealthCheckStatic, this);
    }
    while(true) {

        StringUtils::printInfo("Esperando algum cliente conectar... ");

        sem_wait(&_semaphorClientFD); // Espera a thread que vai lidar com o cliente pegar o clientFD
        _currentClientFD = _serverSocket->acceptConnection();
        if(_currentClientFD == -1) {
            StringUtils::printDanger("Houve um problema ao conectar com o cliente");
            sem_post(&_semaphorClientFD);
            continue;
        }

        StringUtils::printInfo("Cliente conectado...");

        pthread_t clientHandlerThread;
        if (pthread_create(&clientHandlerThread, NULL, &Servidor::handleClientStatic, this) != 0) { // Static func of class, this is necessary to keep the context of the class
            StringUtils::printDanger("Erro ao criar a Thread para lidar com o cliente.");
            sem_post(&_semaphorClientFD);
        }
    }
    
    _serverSocket->closeSocket();
}

void* Servidor::handleClientStatic(void* context) {
    ((Servidor*)context)->handleClient();
    pthread_exit(NULL);
}

void Servidor::handleClient() {
    int n;
    int clientFD = _currentClientFD;
    char buffer[MAX_MSG];

    sem_post(&_semaphorClientFD); // Libera a próxima conexão depois de pegar o clientFD

    Pacote send;
    StringUtils::printWarning("Thread criada para lidar com as requisicoes do cliente");

    memset(buffer, 0, sizeof(buffer));
    while( (n = _serverSocket->receive(clientFD, buffer, MAX_MSG)) > 0 ) {
        Pacote* recebido = new Pacote(buffer);
        StringUtils::printBold(recebido->serializeAsString());
        switch (recebido->getComando())
        {
            case Comando::CONNECT:
                send = handleConnect(recebido->getUsuario(), clientFD);   
                break;
            case Comando::DISCONNECT:
                handleDisconnect(recebido->getUsuario(), clientFD);
                break;
            case Comando::SEND:
                send = handleSend(recebido->getUsuario(), recebido->getTimestamp(), recebido->getPayload(), recebido->getTamanhoPayload());
                sem_post(&_semaphorNotifications);
                break;
            case Comando::FOLLOW:
                send = handleFollow(recebido->getPayload(), recebido->getUsuario());
                //StringUtils::printBold(recebido->serializeAsString());
                break;
            case Comando::GETNOTIFICATIONS:
                sem_post(&_semaphorNotifications);
                break;
            case Comando::CONNECTSERVERTOPOOL:
                sem_wait(&_semaphorPool);
                notifyServersToUpdatePool(recebido->getUsuario(), to_string(clientFD));
                updatePool(recebido->getUsuario(), to_string(clientFD));
                sem_post(&_semaphorPool);
                break;
            case Comando::UPDATEPOOLCHECK:
                if(recebido->getStatus() == Status::OK)
                    StringUtils::printSuccess("Pool atualizado no servidor:" + recebido->getUsuario());
            case Comando::TESTE:
                printPool();
                break;
            default:
                break;
        }
        
        if(recebido->getComando() != Comando::GETNOTIFICATIONS) {
            if(_serverSocket->sendMessage(clientFD, send.serializeAsString().c_str()) == -1)
                StringUtils::printDanger("Erro ao enviar a mensagem de volta");
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

Pacote Servidor::handleConnect(string usuario, int socketDescriptor) {
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
        // Caso esteja com duas conexẽos da ruim
        sem_wait(&_perfis[index]._semaphorePerfil);
        if(_perfis[index]._socketDescriptors.size() == 2) {
            string message = "Numero maximo de conexoes para o usuario excedido";
            send.setStatus(Status::ERROR);
            send.setPayload(message);
            StringUtils::printWarning("Desconectando " + usuario + " no socket " +  to_string(socketDescriptor) + ". Motivo: " + message);
        }
        else { // Caso não, adiciona
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
    if(send.getStatus() == Status::ERROR) {
        _serverSocket->sendMessage(socketDescriptor, send.serializeAsString().c_str());
        StringUtils::printDanger("Fechando socket " + socketDescriptor);
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
    StringUtils::printWarning("Desconectando " + usuario + " no socket " +  to_string(socketDescriptor) + ". Motivo: Usuario deslogado");
    _serverSocket->closeSocket(socketDescriptor);
    pthread_exit(NULL);
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
    // Procura se o usuário está logado já
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

void Servidor::sendNotificacao(std::string from, int idNotificacao) {
    for(std::vector<Perfil>::iterator perfil = _perfis.begin(); perfil != _perfis.end(); perfil++) {
        if(perfil->_usuario == from) {
            sem_wait(&perfil->_semaphorePerfil);
            for(std::vector<Notificacao>::iterator notificacao = perfil->_notificacoesRecebidas.begin(); notificacao != perfil->_notificacoesRecebidas.end(); notificacao++) {
                if(notificacao->_id == idNotificacao) {
                    Pacote* pacote = new Pacote(Tipo::DATA, notificacao->_timestamp, Comando::NOTIFICATION, from, notificacao->_mensagem);
                    for(std::vector<Perfil>::iterator p = _perfis.begin(); p != _perfis.end(); p++) {
                        if(p->_usuario == perfil->_usuario)
                            continue;
                        sem_wait(&p->_semaphorePerfil);
                        for(std::vector<std::pair<std::string,int>>::iterator par = p->_notificacoesPendentes.begin(); par != p->_notificacoesPendentes.end(); par++) {
                            if(par->second == idNotificacao) {
                                bool sent = false;
                                for(int i=0; i<p->_socketDescriptors.size(); i++) {
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
                        sem_post(&p->_semaphorePerfil);
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
}

void Servidor::sendNotificacoes(Perfil to) {
    std::vector<std::pair<std::string, int>> notificacoes;
    sem_wait(&to._semaphorePerfil);
    for(int i=0; i < to._notificacoesPendentes.size(); i++ ) {
        notificacoes.push_back(std::make_pair(to._notificacoesPendentes[i].first, to._notificacoesPendentes[i].second));
    }
    sem_post(&to._semaphorePerfil);
    
    for(int i=0; i < notificacoes.size(); i++ ) {
        sendNotificacao(notificacoes[i].first, notificacoes[i].second);
    }
}

void Servidor::notifyClientsToUpdateServer() {
    Pacote pacote;
    pacote.setComando(Comando::UPDATECONNECTION);
    pacote.setStatus(Status::OK);
    pacote.setPayload(to_string(_serverSocket->getServerFD()));

    sendPacoteToAllClients(pacote);
}

void Servidor::sendPacoteToAllClients(Pacote pacote) {
    for(std::vector<Perfil>::iterator perfil = _perfis.begin(); perfil != _perfis.end(); perfil++) {
        sem_wait(&perfil->_semaphorePerfil);
        for(int i=0; i<perfil->_socketDescriptors.size(); i++) {
            _serverSocket->sendMessage(perfil->_socketDescriptors[i], pacote.serializeAsString().c_str());
        }
        sem_post(&perfil->_semaphorePerfil);
    }
}

void* Servidor::handleHealthCheckStatic(void* context) {
    ((Servidor*)context)->handleHealthCheck();
    pthread_exit(NULL);
}
void Servidor::handleHealthCheck() {
    /*Pacote p;
    p.setStatus(Status::OK);

    _primaryServerSocket->sendMessage()*/
}

void Servidor::updatePool(string serverPID, string serverFD) {
    StringUtils::printWarning("Atualizando Pool");
    _poolServidores.push_back(std::make_pair((pid_t) atoi(serverPID.c_str()), atoi(serverFD.c_str())));
    printPool();
}

void Servidor::notifyServersToUpdatePool(string serverPID, string serverFD) {
    Pacote pacote;
    pacote.setComando(Comando::UPDATEPOOLINSERT);
    pacote.setStatus(Status::OK);
    pacote.setUsuario(serverPID);
    pacote.setPayload(serverFD);

    sendPacoteToAllServers(pacote);
    //sem_wait(&_semaphorUpdatingPool);
}

void Servidor::sendPacoteToAllServers(Pacote p) {
    StringUtils::printInfo("Notificando servidores de atualização do pool");
    for(int i=0; i<_poolServidores.size(); i++) {
        StringUtils::printWarning(to_string(_poolServidores[i].second));
        if(_serverSocket->sendMessage(_poolServidores[i].second, p.serializeAsCharPointer()) == -1)
                StringUtils::printDanger("Erro ao enviar a atualizacao de pool");
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
    puts("./app_servidor [-h | --help] <PORTA> [PORTA_RM_PRIMARIO]");
    puts("Onde <PORTA> e [PORTA_RM_PRIMARIO] estao no intervalo [1, 65535]");
    cout << endl;

    StringUtils::printBold("EXAMPLES");
    puts("./app_servidor 8080 8081");
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

Perfil Servidor::getPerfilByUsername(string username){
    Perfil result;
    result._usuario = "ERRO";
    for(Perfil p : _perfis) {
        if(p._usuario == username){
            result = p;
        }
    }
    return result;
}

void Servidor::gracefullShutDown() {
    saveFile();
    notifyAllConnectedClients();
    sem_destroy(&_semaphorClientFD);
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

void Servidor::notifyAllConnectedClients() {
    Pacote* send = new Pacote();
    send->setComando(Comando::DISCONNECT);
    send->setPayload("O Servidor esta sendo desligado!");
    for(std::vector<Perfil>::iterator perfil = _perfis.begin(); perfil != _perfis.end(); perfil++) {
        sem_wait(&perfil->_semaphorePerfil);
        for(int i=0; i<perfil->_socketDescriptors.size(); i++) {
            int socket = perfil->_socketDescriptors[i];
            _serverSocket->sendMessage(socket, send->serializeAsString().c_str());
        }
        sem_post(&perfil->_semaphorePerfil);
    }
}

void Servidor::printPool() {
    StringUtils::printInfo("Pool atual:");
    for(int i=0; i<_poolServidores.size(); i++) {
        StringUtils::printWithPrefix("FD: " + to_string(_poolServidores[i].second),"PID: " + to_string(_poolServidores[i].first), Color::MAGENTA);
    }
}