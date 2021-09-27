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

    StringUtils::printWarning("Outra Thread criada para lidar com as requisicoes do cliente");

    memset(buffer, 0, sizeof(buffer));
    while( (n = _serverSocket->receive(clientFD, buffer, MAX_MSG)) > 0 ) {
        Pacote* recebido = new Pacote(buffer);
        switch (recebido->getComando())
        {
            case Comando::CONNECT:
                handleConnect(recebido->getUsuario(), clientFD);               
                break;
            case Comando::DISCONNECT:
                handleDisconnect(recebido->getUsuario(), clientFD);
                break;
            case Comando::SEND:
                StringUtils::printBold(recebido->serializeAsString());
                handleSend(recebido->getUsuario(), recebido->getTimestamp(), recebido->getPayload(), recebido->getTamanhoPayload());
                break;
            case Comando::FOLLOW:
                handleFollow(recebido->getPayload(), recebido->getUsuario());
                StringUtils::printBold(recebido->serializeAsString());
                //handleSend(recebido->getUsuario(), recebido->getTimestamp(), recebido->getPayload(), recebido->getTamanhoPayload());
                break;
            default:
                break;
        }

        /*
        StringUtils::printInfo("Mensagem recebida e enviada de volta ao cliente: ");
        StringUtils::printBold(recebido->serializeAsString());
        //strcpy(buffer,StringUtils::removeNewLineAtEnd(buffer));
        if(_serverSocket->sendMessage(clientFD, recebido->serializeAsString().c_str()) == -1)
            StringUtils::printDanger("erro ao enviar a mensagem de volta");
        */
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
void Servidor::handleConnect(string usuario, int socketDescriptor) {
    bool conectado = false;
    int index;
    Pacote* send;
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
            send = new Pacote(Tipo::DATA, Status::ERROR, message);
            StringUtils::printWarning("Desconectando " + usuario + " no socket " +  to_string(socketDescriptor) + ". Motivo: " + message);
        }
        else { // Caso não, adiciona
            _perfis[index]._socketDescriptors.push_back(socketDescriptor);
            send = new Pacote(Tipo::DATA, Status::OK, "Usuario conectado com sucesso");
        }
    }
    else {
        //StringUtils::printBold(usuario);
        //StringUtils::printBold(to_string(socketDescriptor));
        Perfil novoPerfil(usuario, socketDescriptor);
        _perfis.push_back(novoPerfil);
        send = new Pacote(Tipo::DATA, Status::OK, "Usuario conectado com sucesso");
    }

    _serverSocket->sendMessage(socketDescriptor, send->serializeAsString().c_str());
    
    // fecha socket e termina a thread caso usuario tenha sido "deslogado"
    if(send->getStatus() == Status::ERROR) {
        _serverSocket->closeSocket(socketDescriptor);
        pthread_exit(NULL);
    }
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

void Servidor::handleSend(std::string usuario, time_t timestamp, std::string payload, int tamanhoPayload) {
    Notificacao notificacao;
    notificacao._timestamp = timestamp;
    notificacao._mensagem = payload;
    notificacao._tamanho = tamanhoPayload;

    notificacao.printNotificacao();    
}

void Servidor::handleFollow(std::string usuarioSeguido, std::string usuarioSeguidor) {
    bool existeSeguido = false;
    int index;
    Pacote* send;
    StringUtils::printInfo("Usuario " + usuarioSeguidor + " comecou a seguir " + usuarioSeguido);
    //StringUtils::printBold(to_string(perfis.size()));
    // Procura se o usuário está logado já
    for(int i=0; i<_perfis.size(); i++) {
        if(_perfis[i]._usuario == usuarioSeguido) {
            existeSeguido = true;
            index = i;
            _perfis[i]._seguidores.push_back(usuarioSeguidor);
            break;
        }
    }
    if(!existeSeguido) {
        Perfil novoPerfil;
        novoPerfil._usuario = usuarioSeguido;
        novoPerfil._seguidores.push_back(usuarioSeguidor);
        _perfis.push_back(novoPerfil);
    }
    printPerfis();
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
