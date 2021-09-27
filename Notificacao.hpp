#pragma once

#include <string>
#include <time.h>

#include "StringUtils.hpp"

class Notificacao {
    private:
        
    public:
        int _id;
        time_t _timestamp; // Momento em que a notificacao foi recebida
        std::string _mensagem;
        int _tamanho;
        int _quantidadeSeguidoresAReceber;

        Notificacao();
        ~Notificacao();

        void printNotificacao();
};