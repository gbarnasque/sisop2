#pragma once

#include <string>
#include <string.h>
#include <ctime>

enum Tipo {
    DATA,
    COMMAND
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

        void setTipo(Tipo tipo);
        void setTimestamp(time_t timestamp);
        void setPayload(std::string payload);

        char* serialize();
        void deserialize(char* pacoteSerializado);
};