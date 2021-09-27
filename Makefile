CPP = g++
FLAGS = -Wall -g -Werror -pthread
UTILS = ./objects/StringUtils.o ./objects/TCPSocket.o ./objects/Pacote.o ./objects/Notificacao.o ./objects/Perfil.o
OBJSFOLDER = ./objects

all: app_cliente app_servidor

clean:
	rm -rf ${OBJSFOLDER}/*.o app_servidor app_cliente

# Cliente Stuff
app_cliente: ${UTILS} ${OBJSFOLDER}/app_cliente.o ${OBJSFOLDER}/Cliente.o
	${CPP} ${FLAGS} -o app_cliente ${OBJSFOLDER}/app_cliente.o ${OBJSFOLDER}/Cliente.o ${UTILS}

${OBJSFOLDER}/Cliente.o: src/Cliente/Cliente.cpp src/Cliente/Cliente.hpp
	${CPP} -c src/Cliente/Cliente.cpp -o ${OBJSFOLDER}/Cliente.o

${OBJSFOLDER}/app_cliente.o: src/Cliente/app_cliente.cpp
	${CPP} -c src/Cliente/app_cliente.cpp -o ${OBJSFOLDER}/app_cliente.o

# Servidor stuff
app_servidor: ${UTILS} ${OBJSFOLDER}/app_servidor.o ${OBJSFOLDER}/Servidor.o 
	${CPP} ${FLAGS} -o app_servidor ${OBJSFOLDER}/app_servidor.o ${OBJSFOLDER}/Servidor.o ${UTILS}

${OBJSFOLDER}/Servidor.o: src/Servidor/Servidor.cpp src/Servidor/Servidor.hpp
	${CPP} -c src/Servidor/Servidor.cpp -o ${OBJSFOLDER}/Servidor.o

${OBJSFOLDER}/app_servidor.o: src/Servidor/app_servidor.cpp
	${CPP} -c src/Servidor/app_servidor.cpp -o ${OBJSFOLDER}/app_servidor.o

# Utils
${OBJSFOLDER}/StringUtils.o: src/StringUtils/StringUtils.cpp src/StringUtils/StringUtils.hpp
	${CPP} -c src/StringUtils/StringUtils.cpp -o ${OBJSFOLDER}/StringUtils.o

${OBJSFOLDER}/TCPSocket.o: src/TCPSocket/TCPSocket.cpp src/TCPSocket/TCPSocket.hpp
	${CPP} -c src/TCPSocket/TCPSocket.cpp -o ${OBJSFOLDER}/TCPSocket.o

${OBJSFOLDER}/Pacote.o: src/models/Pacote.cpp src/models/Pacote.hpp
	${CPP} -c src/models/Pacote.cpp -o ${OBJSFOLDER}/Pacote.o

${OBJSFOLDER}/Perfil.o: src/models/Notificacao.o src/models/Perfil.cpp src/models/Perfil.hpp
	${CPP} -c src/models/Perfil.cpp -o ${OBJSFOLDER}/Perfil.o

${OBJSFOLDER}/Notificacao.o: src/models/Notificacao.cpp src/models/Notificacao.hpp
	${CPP} -c src/models/Notificacao.cpp -o ${OBJSFOLDER}/Notificacao.o