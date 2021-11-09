#include <iostream>
#include "Pacote.hpp"

Pacote::Pacote() {  
    _tipo = Tipo::DATA;
    _timestamp = time(NULL);
    _comando = Comando::NO;
    _usuario = "";
    _payload = "";
    _tamanhoPayload = 0;
    _status = Status::OK;
}

Pacote::Pacote(Tipo tipo, time_t timestamp, std::string payload) {
    _tipo = tipo;
    _timestamp = timestamp;
    _comando = Comando::NO;
    _usuario = "";
    _payload = payload;
    _tamanhoPayload = _payload.length();
    _status = Status::OK;
}

Pacote::Pacote(Tipo tipo, time_t timestamp, Comando comando, std::string usuario, std::string payload) {
    _tipo = tipo;
    _timestamp = timestamp;
    _comando = comando;
    _usuario = usuario;
    _payload = payload;
    _tamanhoPayload = _payload.length();
    _status = Status::OK;
}

Pacote::Pacote(Tipo tipo, Status status, std::string payload) {
    _tipo = tipo;
    _status = status;
    _payload = payload;
    _tamanhoPayload = _payload.length();
    _timestamp = time(NULL);
    _comando = Comando::NO;
    _usuario = "";
}
Pacote::Pacote(Status status, std::string payload) {
    _status = status;
    _payload = payload;
    _tamanhoPayload = payload.length();
    _tipo = Tipo::DATA;
    _timestamp = time(NULL);
    _comando = Comando::NO;
    _usuario = "";
}

Pacote::Pacote(char* pacoteSerializado) {
    deserialize<char*>(pacoteSerializado);
}

Pacote::Pacote(std::string pacoteSerializado) {
    deserialize<std::string>(pacoteSerializado);
}

void Pacote::setTipo(Tipo tipo) {
    _tipo = tipo;
}
Tipo Pacote::getTipo() {
    return _tipo;
}

void Pacote::setTimestamp(time_t timestamp) {
    _timestamp = timestamp;
}
time_t Pacote::getTimestamp() {
    return _timestamp;
}

void Pacote::setStatus(Status status) {
    _status = status;
}
Status Pacote::getStatus() {
    return _status;
}

void Pacote::setComando(Comando comando) {
    _comando = comando;
}
Comando Pacote::getComando() {
    return _comando;
}

void Pacote::setUsuario(std::string usuario) {
    _usuario = usuario;
}
std::string Pacote::getUsuario() {
    return _usuario;
}

void Pacote::setPayload(std::string payload) {
    _payload = payload;
    _tamanhoPayload = _payload.length();
}
std::string Pacote::getPayload() {
    return _payload;
}
int Pacote:: getTamanhoPayload() {
    return _tamanhoPayload;
}

bool Pacote::getIsAlive() {
    return _isAlive;
}
void Pacote::setIsAlive(bool isAlive) {
    _isAlive = isAlive;
}
void Pacote::setIsAlive(std::string isAlive) {
    _isAlive = (isAlive == "true");
}

bool Pacote::getIsPrimary() {
    return _isPrimary;
}
void Pacote::setIsPrimary(bool isPrimary) {
    _isPrimary = isPrimary;
}
void Pacote::setIsPrimary(std::string isPrimary) {
    _isPrimary = (isPrimary == "true");
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

std::string Pacote::generateString(string identificador, string s, bool end) {
    string newString;
    newString.append("\"").append(identificador).append("\":");
    newString.append("\"").append(s).append("\"");

    if(!end)
        newString.append(",");

    return newString;
}

std::string Pacote::generateBool(string identificador, bool b, bool end) {
    string newString;
    newString.append("\"").append(identificador).append("\":");
    newString.append(b ? "true" : "false");

    if(!end)
        newString.append(",");

    return newString;
} 

char* Pacote::serializeAsCharPointer() {
    std::string pacoteSerializadoString = serializeAsString();
    char* pacoteSerializado;
    
    pacoteSerializado = (char*)malloc(sizeof(char)*pacoteSerializadoString.length()+1);
    
    strcpy(pacoteSerializado, pacoteSerializadoString.c_str());

    return pacoteSerializado;
}

string Pacote::serializeAsString() {
    std::string pacoteSerializadoString;
     
    pacoteSerializadoString.append("{");
    pacoteSerializadoString.append(generateInt<Tipo>("tipo", _tipo));
    pacoteSerializadoString.append(generateInt<Status>("status", _status));
    pacoteSerializadoString.append(generateInt<time_t>("timestamp", _timestamp));
    pacoteSerializadoString.append(generateInt<Comando>("comando", _comando));
    pacoteSerializadoString.append(generateString("usuario", _usuario));
    pacoteSerializadoString.append(generateBool("isAlive", _isAlive));
    pacoteSerializadoString.append(generateBool("isPrimary", _isPrimary));
    pacoteSerializadoString.append(generateString("payload", _payload, true));
    pacoteSerializadoString.append("}");

    return pacoteSerializadoString;
}

template <typename T>
void Pacote::deserialize(T pacoteSerializado) {
    std::string nome;
    std::string valor;
    std::string pacoteSerializadoString(pacoteSerializado);
   
    if(pacoteSerializadoString.length() != 2) {
        while(pacoteSerializadoString.length() != 1 && pacoteSerializadoString.length() != 0) {
            std::size_t aspasAntes = pacoteSerializadoString.find_first_of("\"");
            std::size_t aspasPos = pacoteSerializadoString.find_first_of("\"", aspasAntes+1);
            nome = pacoteSerializadoString.substr(aspasAntes + 1, aspasPos - aspasAntes - 1);

           
            pacoteSerializadoString = pacoteSerializadoString.substr(aspasPos+2);
            
            std::size_t terminador = pacoteSerializadoString.find_first_of(",}");
            valor = pacoteSerializadoString.substr(0, terminador);

            //*
            if(nome == "tipo") {
                setTipo(static_cast<Tipo>(std::stoi(valor)));
            }
            else if(nome == "timestamp") {  
                setTimestamp(std::stol(valor));
            }
            else if(nome == "payload") {
                valor.erase(0, 1); // Remove " inicial
                valor.erase(valor.end()-1); // Remove " final
                setPayload(valor);
            }
            else if(nome == "usuario") {
                valor.erase(0, 1); // Remove " inicial
                valor.erase(valor.end()-1); // Remove " final
                setUsuario(valor);
            }
            else if(nome == "comando") {
                setComando(static_cast<Comando>(stoi(valor)));
            }
            else if(nome == "status") {
                setStatus(static_cast<Status>(stoi(valor)));
            }
            else if(nome == "isAlive") {
                setIsAlive(valor);
            }
            else if(nome == "isPrimary") {
                setIsPrimary(valor);
            }

            pacoteSerializadoString = pacoteSerializadoString.substr(terminador);
        }
    }
}

std::vector<Pacote> Pacote::getMultiplosPacotes(const char* pacotesSerializados) {
    std::vector<Pacote> pacotes;
    std::string pacotesSerializadosString(pacotesSerializados);

    std::size_t terminador;
    while((terminador = pacotesSerializadosString.find_first_of("}")) != std::string::npos ) {
        Pacote novoPacote(pacotesSerializadosString.substr(0, terminador+1));
        pacotes.push_back(novoPacote);
        pacotesSerializadosString = pacotesSerializadosString.substr(terminador+1);
    }
    
    return pacotes;
}

Comando Pacote::getComandoFromLine(std::string line) {
    size_t espaco = line.find_first_of(" ");
    if(espaco == std::string::npos) {
        return Comando::NO;
    }
    std::string comando = line.substr(0, espaco);
    if(comando == "SEND") {
        return Comando::SEND;
    }
    else if(comando == "FOLLOW") {
        return Comando::FOLLOW;
    }
    else if(comando == "TESTE") {
        return Comando::TESTE;
    }
    else if(comando == "POOL"){
        return Comando::POOL;
    }
    else if(comando == "PERFIS") {
        return Comando::PERFIS;
    }
    else {
        return Comando::NO;
    }
}

std::string Pacote::removeComandoFromLine(std::string line) {
    size_t espaco = line.find_first_of(" ");
    std::string newLine = line.substr(espaco+1);
    return newLine;
}
