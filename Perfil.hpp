#pragma once
#include <string>
#include <vector>
#include <utility>

#include "Notificacao.hpp"
#include "StringUtils.hpp"

using namespace std;

class Perfil {
    private:
        
    public:
        string _usuario;
        vector<string> _seguidores;
        vector<Notificacao> _notificacoesRecebidas;
        vector<pair<string, int>> _notificacoesPendentes; // <Perfil a receber, id notificacao>
        vector<int> _socketDescriptors;

        Perfil();
        ~Perfil();
        Perfil(string usuario, int socketDescriptor);
        

};