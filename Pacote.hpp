#pragma once

#include <string>
#include <string.h>
#include <ctime>

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
    NO = -1
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
        ~Pacote();
        Pacote(Tipo tipo, time_t timestamp, std::string payload);
        Pacote(Tipo tipo, time_t timestamp, Comando comando, std::string usuario, std::string payload = "");
        Pacote(char* pacoteSerializado);
        Pacote(std::string pacoteSerializado);

        void setTipo(Tipo tipo);
        Tipo getTipo();
        void setTimestamp(time_t timestamp);
        time_t getTimestamp();
        void setComando(Comando comando);
        Comando getComando();
        void setUsuario(std::string usuario);
        std::string getUsuario();
        void setPayload(std::string payload);
        std::string getPayload();
        int getTamanhoPayload();

        char* serialize(char* message);
        std::string serialize();
        void deserialize(char* pacoteSerializado);
        void deserialize(std::string pacoteSerializadoString);
};