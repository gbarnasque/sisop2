#pragma once

#include <string>
#include <string.h>
#include <ctime>
#include <vector>
#include "../StringUtils/StringUtils.hpp"

enum Tipo {
    DATA,
    COMMAND,
    CLIENTE,
    SERVIDOR
};

enum Comando {
    CONNECT,
    DISCONNECT,
    SEND,
    FOLLOW,
    NOTIFICATION,
    GETNOTIFICATIONS,
    TESTE,
    POOL,
    PERFIS,
    NO = -1
};

enum Status {
    OK,
    ERROR
};

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

        static std::vector<Pacote> getMultiplosPacotes(const char* pacotesSerializados);
        static Comando getComandoFromLine(std::string line);
        static std::string removeComandoFromLine(std::string line);
};