#include "Perfil.hpp"

Perfil::~Perfil() {
    sem_destroy(&_semaphorePerfil);
}

Perfil::Perfil() {
    sem_init(&_semaphorePerfil, 0, 1);
}

Perfil::Perfil(string usuario) {
    _usuario = usuario;
    sem_init(&_semaphorePerfil, 0, 1);
}

Perfil::Perfil(string usuario, int socketDescriptor) {
    _usuario = usuario;
    _socketDescriptors.push_back(socketDescriptor);
    sem_init(&_semaphorePerfil, 0, 1);
}

void Perfil::printPerfil() {
    StringUtils::printWithPrefix(_usuario, "usuario:", Color::RED);
    StringUtils::printWithPrefix("", "Seguidores: ", Color::BLUE);
    if(_seguidores.size() > 0) {
        std::string seguidores = "";
        for(int i=0; i<_seguidores.size(); i++) {
            seguidores.append(_seguidores[i]).append(", ");
        }
        StringUtils::printLine(seguidores);
    }
    else {
        StringUtils::printLine("Nenhum");
    }

    StringUtils::print("Notificacoes pendentes: ", Color::CYAN, true);
    //StringUtils::printWithPrefix("", "Notificacoes pendentes: ", Color::CYAN);
    if(_notificacoesPendentes.size() > 0) {
        std::string notificacoesPendentes = "";
        for(int i=0; i<_notificacoesPendentes.size(); i++) {
            notificacoesPendentes.append("(").append(_notificacoesPendentes[i].first).append(",").append(to_string(_notificacoesPendentes[i].second)).append("), ");
        }
        StringUtils::printLine(notificacoesPendentes);
    }
    else {
        StringUtils::printLine("Nenhuma");
    }
    
    StringUtils::printWithPrefix("", "Sockets: ", Color::BRIGHT_MAGENTA);
    if(_socketDescriptors.size() > 0) {
        std::string sockets = "";
        for(int i=0; i<_socketDescriptors.size(); i++) {
            sockets.append(to_string(_socketDescriptors[i])).append(", ");
        }
        StringUtils::printLine(sockets);
    }
    else {
        StringUtils::printLine("Nenhum");
    }

    StringUtils::printWithPrefix("", "Notificacoes Recebidas: ", Color::GREEN);
    if(_notificacoesRecebidas.size() > 0) {
        for(int i=0 ;i<_notificacoesRecebidas.size(); i++) {
            _notificacoesRecebidas[i].printNotificacao();
        }
    }
    else {
        StringUtils::printLine("Nenhuma");
    }
}