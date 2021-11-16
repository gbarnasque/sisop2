#pragma once
#include <string>
#include <vector>
#include <utility>
#include <semaphore.h>

#include "../models/Notificacao.hpp"
#include "../StringUtils/StringUtils.hpp"

using namespace std;

typedef struct server_perfil {
    pid_t PID;
    string Ip;
    string Port;
    int FD;
    bool isAlive;
} ServerPerfil;

class Perfil {
    private:
        
    public:
        string _usuario;
        vector<string> _seguidores;
        vector<Notificacao> _notificacoesRecebidas;
        vector<pair<string, int>> _notificacoesPendentes; // <Perfil a receber, id notificacao>
        vector<int> _socketDescriptors;
        sem_t _semaphorePerfil;

        Perfil();
        ~Perfil();
        Perfil(string usuario);
        Perfil(string usuario, int socketDescriptor);

        
        void printPerfil();
};