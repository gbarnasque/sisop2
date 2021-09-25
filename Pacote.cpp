#include <iostream>

#include "StringUtils.hpp"
#include "Pacote.hpp"

Pacote::Pacote() {  
    _tipo = Tipo::COMMAND;
    _timestamp = time(NULL);
    _payload = "";
    _tamanhoPayload = 0;
}

Pacote::Pacote(Tipo tipo, time_t timestamp, std::string payload) {
    _tipo = tipo;
    _timestamp = timestamp;
    _payload = payload;
    _tamanhoPayload = _payload.length();
}

void Pacote::setTipo(Tipo tipo) {
    _tipo = tipo;
}

void Pacote::setTimestamp(time_t timestamp) {
    _timestamp = timestamp;
}

void Pacote::setPayload(std::string payload) {
    _payload = payload;
    _tamanhoPayload = _payload.length();
}

template <typename T>
std::string Pacote::generateInt(string identificador, T i, bool end) {
    string newString;
    newString.append("\"").append(identificador).append("\":");
    newString.append(std::to_string(i));
    
    if(!end)
        newString.append(",");
    
    return newString;    
}
/*
std::string Pacote::generateInt(string identificador, int i, bool end) {
    string newString;
    newString.append("\"").append(identificador).append("\":");
    newString.append(std::to_string(i));
    newString.append(",");

    if(!end)
        newString.append(",");
    
    return newString;    
}

std::string Pacote::generateInt(string identificador, Tipo t, bool end) {
    string newString;
    newString.append("\"").append(identificador).append("\":");
    newString.append(std::to_string(t));
    newString.append(",");

    if(!end)
        newString.append(",");
    
    return newString;    
}

std::string Pacote::generateInt(string identificador, time_t t, bool end) {
    string newString;
    newString.append("\"").append(identificador).append("\":");
    newString.append(std::to_string(t));
    newString.append(",");

    if(!end)
        newString.append(",");
    
    return newString;    
}
*/
std::string Pacote::generateString(string identificador, string s, bool end) {
    string newString;
    newString.append("\"").append(identificador).append("\":");
    newString.append("\"").append(s).append("\"");

    if(!end)
        newString.append(",");

    return newString;
} 

char* Pacote::serialize() {
    StringUtils::printLine("Serializando Pacote");
    std::string pacoteSerializadoString;
    char* pacoteSerializado;
    
    
    pacoteSerializadoString.append("{");
    pacoteSerializadoString.append(generateInt<Tipo>("tipo", _tipo));
    pacoteSerializadoString.append(generateInt<time_t>("timestamp", _timestamp));
    pacoteSerializadoString.append(generateString("payload", _payload, true));
    pacoteSerializadoString.append("}");

    StringUtils::printLine(pacoteSerializadoString);
    pacoteSerializado = (char*)malloc(sizeof(char)*pacoteSerializadoString.length() + 1);
    strcpy(pacoteSerializado, pacoteSerializadoString.c_str());

    return pacoteSerializado;
}

void Pacote::deserialize(char* pacoteSerializado) {
    
}