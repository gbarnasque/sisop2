#pragma once

#include <string>
#include <string.h>
#include <ctime>
#include <vector>
#include "../StringUtils/StringUtils.hpp"

enum Tipo {
    DATA,
    COMMAND
};

enum Comando {
    CONNECT,
    DISCONNECT,
    SEND,
    FOLLOW,
    NOTIFICATION,
    GETNOTIFICATIONS,
    TESTE,
    NO = -1
};

enum Status {
    OK,
    ERROR
};

/*const char* TipoToString(Tipo t) {
    switch (t)
    {
    case DATA: return "DATA";
    case COMMAND: return "COMMANDO";
    default: return "";
    }
}*/

class Pacote {
    private:
        Tipo _tipo;
        time_t _timestamp;
        Status _status;
        Comando _comando;
        std::string _usuario;
        std::string _payload;
        int _tamanhoPayload;

        template <typename T>
        std::string generateInt(string identificador, T i, bool end = false);
        std::string generateString(string identificador, string s, bool end = false);
         //std::string generateInt(string identificador, int t, bool end);
        //std::string generateInt(string identificador, time_t t, bool end);
        //std::string generateInt(string identificador, Tipo t, bool end);
        

    public:
        Pacote();
        //~Pacote();
        Pacote(Tipo tipo, time_t timestamp, std::string payload);
        Pacote(Tipo tipo, time_t timestamp, Comando comando, std::string usuario, std::string payload = "");
        Pacote(char* pacoteSerializado);
        Pacote(std::string pacoteSerializado);
        Pacote(Tipo tipo, Status status, std::string payload);
        Pacote(Status status, std::string payload);

        void setTipo(Tipo tipo);
        Tipo getTipo();
        void setTimestamp(time_t timestamp);
        time_t getTimestamp();
        void setStatus(Status status);
        Status getStatus();
        void setComando(Comando comando);
        Comando getComando();
        void setUsuario(std::string usuario);
        std::string getUsuario();
        void setPayload(std::string payload);
        std::string getPayload();
        int getTamanhoPayload();

        char* serializeAsCharPointer();
        std::string serializeAsString();
        template <typename T>
        void deserialize(T pacoteSerializado);
        //void deserialize(std::string pacoteSerializadoString);

        static std::vector<Pacote> getMultiplosPacotes(const char* pacotesSerializados);
};