#include "Notificacao.hpp"

Notificacao::~Notificacao() {
    
}

Notificacao::Notificacao() {
    _id = -1;
    _timestamp = time(NULL);
    _mensagem = "";
    _tamanho = 0;
    _quantidadeSeguidoresAReceber = 0;
}

void Notificacao::printNotificacao() {
    StringUtils::printWithPrefix(to_string(_id), "id:", Color::NONE);
    StringUtils::printWithPrefix(to_string(_timestamp), "timestamp:", Color::NONE);
    StringUtils::printWithPrefix(_mensagem, "mensagem:", Color::NONE);
    StringUtils::printWithPrefix(to_string(_tamanho), "tamanho da mensagem:", Color::NONE);
    StringUtils::printWithPrefix(to_string(_quantidadeSeguidoresAReceber), "quantidade de seguidores a receber:", Color::NONE);
}