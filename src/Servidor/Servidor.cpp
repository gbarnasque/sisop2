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
    sem_init(&_semaphorClientFD, 0, 1);
    _GLOBAL_NOTIFICACAO_ID = 0;
    _saveFileName = "save.csv";
    fillFromFile();
}

void Servidor::start() {
    StringUtils::printSuccess("Servidor iniciado");
    while(true) {

        StringUtils::printInfo("Esperando algum cliente conectar... ");

        sem_wait(&_semaphorClientFD); // Espera a thread que vai lidar com o cliente pegar o clientFD
        _currentClientFD = _serverSocket->acceptConnection();
        if(_currentClientFD == -1) {
            StringUtils::printDanger("Houve um problema ao conectar com o cliente");
            continue;
        }

        StringUtils::printInfo("Cliente conectado...");

        pthread_t clientHandlerThread;
        if (pthread_create(&clientHandlerThread, NULL, &Servidor::handleClientStatic, this) != 0) { // Static func of class, this is necessary to keep the context of the class
        //if (pthread_create(&clientHandlerThread, NULL, (THREADFUNCPTR) &Servidor::handleClient, this) == 0) { // Pointer to func
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
     //StringUtils::printInfo("Entered thread");
    int n;
    int clientFD = _currentClientFD;
    char buffer[MAX_MSG];

    sem_post(&_semaphorClientFD); // Libera a próxima conexão depois de pegar o clientFD

    Pacote send;
    StringUtils::printWarning("Outra Thread criada para lidar com as requisicoes do cliente");

    memset(buffer, 0, sizeof(buffer));
    while( (n = _serverSocket->receive(clientFD, buffer, MAX_MSG)) > 0 ) {
        Pacote* recebido = new Pacote(buffer);
        switch (recebido->getComando())
        {
            case Comando::CONNECT:
                send = handleConnect(recebido->getUsuario(), clientFD);       
                break;
            case Comando::DISCONNECT:
                handleDisconnect(recebido->getUsuario(), clientFD);
                break;
            case Comando::SEND:
                StringUtils::printBold(recebido->serializeAsString());
                send = handleSend(recebido->getUsuario(), recebido->getTimestamp(), recebido->getPayload(), recebido->getTamanhoPayload());
                break;
            case Comando::FOLLOW:
                send = handleFollow(recebido->getPayload(), recebido->getUsuario());
                StringUtils::printBold(recebido->serializeAsString());
                //handleSend(recebido->getUsuario(), recebido->getTimestamp(), recebido->getPayload(), recebido->getTamanhoPayload());
                break;
            case Comando::GETNOTIFICATIONS:
                sendNotificacoes(recebido->getUsuario());
                break;
            case Comando::TESTE:
                //sendNotificacao("@teste");
                printPerfis();
                break;
            default:
                break;
        }
        
        /*
        StringUtils::printInfo("Mensagem recebida e enviada de volta ao cliente: ");
        StringUtils::printBold(recebido->serializeAsString());
        //strcpy(buffer,StringUtils::removeNewLineAtEnd(buffer));
        */
        if(recebido->getComando() != Comando::GETNOTIFICATIONS) {
            if(_serverSocket->sendMessage(clientFD, send.serializeAsString().c_str()) == -1)
                StringUtils::printDanger("erro ao enviar a mensagem de volta");
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

// Talvez botar semaforo aqui
Pacote Servidor::handleConnect(string usuario, int socketDescriptor) {
    bool conectado = false;
    int index;
    Pacote send(Tipo::DATA, Status::OK, "Usuario conectado com sucesso!");
    StringUtils::printInfo("Lidando com a conexao do usuario " + usuario + " no socket " + to_string(socketDescriptor));
    //StringUtils::printBold(to_string(perfis.size()));
    // Procura se o usuário está logado já
    for(int i=0; i<_perfis.size(); i++) {
        if(_perfis[i]._usuario == usuario) {
            conectado = true;
            index = i;
            break;
        }
    }
    //StringUtils::printBold(to_string(conectado));
    if(conectado) {
        //StringUtils::printWithPrefix(to_string(perfis[index]._socketDescriptors.size()), "Quantidade de conexoes: ", Color::NONE);
        // Caso esteja com duas conexẽos da ruim
        if(_perfis[index]._socketDescriptors.size() == 2) {
            string message = "Numero maximo de conexoes para o usuario excedido";
            send.setStatus(Status::ERROR);
            send.setPayload(message);
            StringUtils::printWarning("Desconectando " + usuario + " no socket " +  to_string(socketDescriptor) + ". Motivo: " + message);
        }
        else { // Caso não, adiciona
            _perfis[index]._socketDescriptors.push_back(socketDescriptor);
        }
    }
    else {
        //StringUtils::printBold(usuario);
        //StringUtils::printBold(to_string(socketDescriptor));
        Perfil novoPerfil(usuario, socketDescriptor);
        _perfis.push_back(novoPerfil);
    }

    // fecha socket e termina a thread caso usuario tenha sido "deslogado"
    if(send.getStatus() == Status::ERROR) {
        _serverSocket->sendMessage(socketDescriptor, send.serializeAsString().c_str());
        StringUtils::printDanger("Fechando socket");
        _serverSocket->closeSocket(socketDescriptor);
        pthread_exit(NULL);
    }

    return send;
}

void Servidor::handleDisconnect(string usuario, int socketDescriptor) {
    for(vector<Perfil>::iterator perfil = _perfis.begin(); perfil != _perfis.end(); perfil++) {
        if(perfil->_usuario == usuario) {
            for(vector<int>::iterator socket = perfil->_socketDescriptors.begin(); socket != perfil->_socketDescriptors.end(); socket++) {
                if(*socket == socketDescriptor) {
                    //StringUtils::printInfo("Desconectando cliente com MAIS de um socket");
                    perfil->_socketDescriptors.erase(socket);
                    break;
                }
            }
            break;
        }
    }
    StringUtils::printWarning("Desconectando " + usuario + " no socket " +  to_string(socketDescriptor) + ". Motivo: Usuario deslogado");
    _serverSocket->closeSocket(socketDescriptor);
    pthread_exit(NULL);
}

Pacote Servidor::handleSend(std::string usuario, time_t timestamp, std::string payload, int tamanhoPayload) {
    Notificacao notificacao;
    notificacao._id = _GLOBAL_NOTIFICACAO_ID++;
    notificacao._timestamp = timestamp;
    notificacao._mensagem = payload;
    notificacao._tamanho = tamanhoPayload;    

    std::vector<string> seguidores;
    for(std::vector<Perfil>::iterator perfil = _perfis.begin(); perfil != _perfis.end(); perfil++) {
        if(perfil->_usuario == usuario) {
            notificacao._quantidadeSeguidoresAReceber = perfil->_seguidores.size();
            notificacao.printNotificacao();
            perfil->_notificacoesRecebidas.push_back(notificacao);
            seguidores = perfil->_seguidores;
            break;
        }
    }

    for(std::vector<Perfil>::iterator perfil = _perfis.begin(); perfil != _perfis.end(); perfil++) {
        for(std::vector<std::string>::iterator seguidor = seguidores.begin(); seguidor != seguidores.end(); seguidor++) {
            if(perfil->_usuario == *seguidor) {
                perfil->_notificacoesPendentes.push_back(std::make_pair(usuario, notificacao._id));
                break;
            }
        }
    }

    return sendNotificacao(usuario, notificacao._id);
}

Pacote Servidor::handleFollow(std::string usuarioSeguido, std::string usuarioSeguidor) {
    bool existeSeguido = false;
    bool jaSegue = false;
    Pacote send(Tipo::DATA, Status::OK, "Seguindo usuario " + usuarioSeguido);
    StringUtils::printInfo("Usuario " + usuarioSeguidor + " comecou a seguir " + usuarioSeguido);
    // Procura se o usuário está logado já
    for(std::vector<Perfil>::iterator perfil = _perfis.begin(); perfil != _perfis.end(); perfil++) {
        if(perfil->_usuario == usuarioSeguido) {
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
                perfil->_seguidores.push_back(usuarioSeguidor);
            }
            break;
        }
    }
    if(!existeSeguido) {
        Perfil novoPerfil;
        novoPerfil._usuario = usuarioSeguido;
        novoPerfil._seguidores.push_back(usuarioSeguidor);
        _perfis.push_back(novoPerfil);
    }

    return send;
}

Pacote Servidor::sendNotificacao(std::string from, int idNotificacao) {
    Pacote p;
    p.setStatus(Status::OK);
    p.setComando(Comando::NO);
    p.setPayload("Mensagem enviada com sucesso!");
    for(std::vector<Perfil>::iterator perfil = _perfis.begin(); perfil != _perfis.end(); perfil++) {
        if(perfil->_usuario == from) {
            StringUtils::printInfo("Usuario encontrado");
            for(std::vector<Notificacao>::iterator notificacao = perfil->_notificacoesRecebidas.begin(); notificacao != perfil->_notificacoesRecebidas.end(); notificacao++) {
                if(notificacao->_id == idNotificacao) {
                    Pacote* pacote = new Pacote(Tipo::DATA, notificacao->_timestamp, Comando::NOTIFICATION, from, notificacao->_mensagem);
                    StringUtils::printInfo("Notificacao encontrada");
                    for(std::vector<Perfil>::iterator p = _perfis.begin(); p != _perfis.end(); p++) {
                        StringUtils::printInfo("Perfil sendo visistado");
                        p->printPerfil();
                        for(std::vector<std::pair<std::string,int>>::iterator par = p->_notificacoesPendentes.begin(); par != p->_notificacoesPendentes.end(); par++) {
                            if(par->second == idNotificacao) {
                                StringUtils::printInfo("Pendencia encontrada");
                                bool sent = false;
                                for(int i=0; i<p->_socketDescriptors.size(); i++) {
                                    _serverSocket->sendMessage(p->_socketDescriptors[i], pacote->serializeAsString().c_str());
                                    sent = true;
                                    StringUtils::printInfo("Pendencia enviada");
                                }
                                if(sent) {
                                    StringUtils::printInfo("Notificacao pendente sendo apagada e diminuindo numero da not");
                                    StringUtils::printInfo("Perfis Antes");
                                    printPerfis();
                                    p->_notificacoesPendentes.erase(par);
                                    notificacao->_quantidadeSeguidoresAReceber--;
                                    StringUtils::printInfo("Perfis Depois");
                                    printPerfis();
                                    StringUtils::printBold("");
                                    notificacao->printNotificacao();
                                    break;
                                }
                            }
                        }
                    }
                    if(notificacao->_quantidadeSeguidoresAReceber == 0) {
                        perfil->_notificacoesRecebidas.erase(notificacao);
                        break;
                    }
                }
                
            }
            break;
        }
    }
    return p;
}

void Servidor::sendNotificacoes(std::string to) {
    std::vector<std::pair<std::string, int>> notificacoes;
    for(Perfil& perfil : _perfis) {
        if(perfil._usuario == to) {
            for(int i=0; i < perfil._notificacoesPendentes.size(); i++ ) {
                notificacoes.push_back(std::make_pair(perfil._notificacoesPendentes[i].first, perfil._notificacoesPendentes[i].second));
            }
        }
    }
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
        for(int i=0; i<perfil->_socketDescriptors.size(); i++) {
            int socket = perfil->_socketDescriptors[i];
            _serverSocket->sendMessage(socket, send->serializeAsString().c_str());
        }
    }
}