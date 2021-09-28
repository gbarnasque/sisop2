#include "Cliente.hpp"

Cliente::Cliente(char* serverIp, char* serverPort, char* user) {
    char recvLine[MAX_MSG];

    _socket = new TCPSocket(serverIp, serverPort);
    
    if(!_socket->connectSocket()) {
        StringUtils::printDanger("Problema ao conectar ao servidor");
        exit(3);
    }
    std::string usuario(user);
    _usuario = usuario;
    Pacote* enviado = new Pacote(Tipo::COMMAND, time(NULL), Comando::CONNECT, usuario);
    //exit(0);
    _socket->sendMessage(enviado->serializeAsString().c_str());
    
    memset(recvLine, 0, sizeof(recvLine));
    _socket->receive(recvLine, MAX_MSG);

    Pacote* recebido = new Pacote(recvLine);
    if(recebido->getStatus() == Status::OK) {
        StringUtils::printSuccess(recebido->getPayload());
    }
    else {
        StringUtils::printDanger(recebido->getPayload());
        _socket->closeSocket();
        exit(4);
    }
    
    enviado->setComando(Comando::GETNOTIFICATIONS);
    _socket->sendMessage(enviado->serializeAsString().c_str());
}

void Cliente::handleExit() {
    StringUtils::printWarning("Saindo do aplicativo cliente");
    Pacote* p = new Pacote();
    p->setComando(Comando::DISCONNECT);
    p->setStatus(Status::OK);
    p->setUsuario(_usuario);
    _socket->sendMessage(p->serializeAsString().c_str());
    _socket->closeSocket();
    exit(0);
}
/*
void* Cliente::HandlePacotesStatic(void* context) {
    ((Cliente*)context)->HandlePacotes();
    pthread_exit(NULL);
}

void Cliente::HandlePacotes() {
    //sem_t pacoteHandler;
    //sem_init(&pacoteHandler, 0, 1);
    StringUtils::printInfo("[HANDLEPACOTES] Thread3 iniciada!");

    while(true) {
        //sem_wait(&pacoteHandler);
        while(_pacotes.size() != 0) {
            Pacote p = _pacotes[0];
            StringUtils::printWarning(p.serializeAsString());
            if(p.getStatus() == Status::OK){
                switch (p.getComando())
                {
                    case Comando::NOTIFICATION:
                        StringUtils::printWithRandomPrefixColor(p.getPayload(), p.getUsuario() + ":");
                        break;
                    case Comando::DISCONNECT:
                        StringUtils::printDanger(p.getPayload());
                        handleExit();
                        break;
                    default:
                        //exit(-1);
                        if(p.getPayload().size() != 0);
                            StringUtils::printSuccess(p.getPayload());
                        break;
                }
            } 
            else {
                if(p.getPayload().size() != 0);
                    StringUtils::printDanger(p.getPayload());
            }
            _pacotes.erase(_pacotes.begin());
        }
    }
}
*/

void* Cliente::receiveNotificationsStatic(void* context){
    ((Cliente*)context)->receiveNotifications();
    pthread_exit(NULL);
}

//recebe as notificações enviadas pelo servidor dos
//perfis que o usuário segue e as imprime na tela
void Cliente::receiveNotifications(){

    StringUtils::printInfo("[RECEIVENOTIFICATIONS] Thread2 iniciada!");
    
    char rcvLine[MAX_MSG];
    memset(rcvLine, 0, sizeof(rcvLine));
    while(_socket->receive(rcvLine, MAX_MSG) != -1) {
        std::vector<Pacote> pacotes = Pacote::getMultiplosPacotes(rcvLine);
        
        for(std::vector<Pacote>::iterator pacote = pacotes.begin(); pacote != pacotes.end(); pacote++) {
            //StringUtils::printWarning(pacote->serializeAsString());
            if(pacote->getStatus() == Status::OK){
                switch (pacote->getComando())
                {
                    case Comando::NOTIFICATION:
                        StringUtils::printWithRandomPrefixColor(pacote->getPayload(), pacote->getUsuario() + ":");
                        break;
                    case Comando::DISCONNECT:
                        StringUtils::printDanger(pacote->getPayload());
                        handleExit();
                        break;
                    default:
                        //exit(-1);
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
        
        //StringUtils::printInfo("[RECEIVENOTIFICATIONS] Mensagem recebida do servidor:");
        
        //StringUtils::printBold(p->serializeAsString());
        
        memset(rcvLine, 0, sizeof(rcvLine));
    }
    _socket->closeSocket();
    StringUtils::printDanger("O servidor encerrou a conexão");
    exit(4);
}

void* Cliente::ProcessKeyboardInputStatic(void* context){
    ((Cliente*)context)->ProcessKeyboardInput();
    pthread_exit(NULL);
}

//Espera em busy wait pelo input do teclado do usuário
void Cliente::ProcessKeyboardInput(){
    StringUtils::printInfo("[PROCESSKEYBOARDINPUT] Esperando pelo input do usuario...");
    while(fgets(sendLine, MAX_MSG, stdin) != NULL) {
        Pacote* send;

        StringUtils::removeNewLineAtEnd(sendLine);
        std::string sendLineString(sendLine);
        Comando comando = getComandoFromLine(sendLineString);
        sendLineString = removeComandoFromLine(sendLineString);
        switch (comando)
        {
            case Comando::FOLLOW:
                if(!lineEstaOK(sendLineString, comando)) {
                    StringUtils::printDanger("O perfil a seguir deve conter entre 4 e 20 caracteres e comecar com @. E, lembre-se, voce nao pode se seguir");
                    break;
                }

            case Comando::SEND:
                if(!lineEstaOK(sendLineString, comando)) {
                    StringUtils::printDanger("Sua mensagem possui mais do que os 128 caracteres permitidos");
                    break;
                }
            case Comando::TESTE:    
                send = new Pacote(Tipo::COMMAND, time(NULL), comando, _usuario, sendLineString);
                _socket->sendMessage(send->serializeAsString().c_str());
                break;
            
            case Comando::NO:
            default:
                StringUtils::printWarning("Comando nao reconhecido, os comando disponiveis sao \"SEND <mensagem>\" e \"FOLLOW <@usuario>\"");
                break;
        }
        memset(sendLine, 0, sizeof(sendLine));
        StringUtils::printInfo("Esperando pelo input do usuario...");
    }
    handleExit();
}

//Cria duas threads para o funcionamento do cliente
//A primeira detecta input do usuário e o processa, enviando-o para o servidor conforme necessário
//A segunda espera receber pacotes do servidor e imprime as mensagens na tela
void Cliente::interact() {

    pthread_t thread1, thread2;

    /* Create independent threads each of which will execute function */

    pthread_create(&thread1, NULL, Cliente::ProcessKeyboardInputStatic, this);
    pthread_create(&thread2, NULL, Cliente::receiveNotificationsStatic, this);

     /* Wait till threads are complete before main continues. Unless we  */
     /* wait we run the risk of executing an exit which will terminate   */
     /* the process and all threads before the threads have completed.   */

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
}


Comando Cliente::getComandoFromLine(std::string line) {
    size_t espaco = line.find_first_of(" ");
    if(espaco == std::string::npos) {
        return Comando::NO;
    }
    std::string comando = line.substr(0, espaco);
    if(comando == "SEND") {
        return Comando::SEND;
    }
    else if(comando == "FOLLOW") {
        return Comando::FOLLOW;
    }
    else if(comando == "TESTE") {
        return Comando::TESTE;
    }
    else {
        return Comando::NO;
    }
}

std::string Cliente::removeComandoFromLine(std::string line) {
    size_t espaco = line.find_first_of(" ");
    std::string newLine = line.substr(espaco+1);
    return newLine;
}

bool Cliente::lineEstaOK(std::string line, Comando c) {
    switch (c)
    {
        case Comando::FOLLOW:
            return (line.length() > 4 && line.length() < 20 && line[0] == '@' && line != _usuario);
        case Comando::SEND:
            return (line.length() <= 128);
        default:
            return false;
    }
}

void Cliente::help() {
    std::cout << std::endl;
    StringUtils::printBold("USAGE");
    puts("./app_cliente [-h | --help] <PERFIL> <IP_SERVIDOR> <PORTA>");
    puts("Onde <PERFIL> deve começar com @ e conter entre 4 e 20 caracteres");
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