#include "Perfil.hpp"

Perfil::~Perfil() {
    
}


Perfil::Perfil(string usuario, int socketDescriptor) {
    _usuario = usuario;
    _socketDescriptors.push_back(socketDescriptor);
}