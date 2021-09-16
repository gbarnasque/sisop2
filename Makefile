CPP = g++
FLAGS = -Wall -g -Werror -pthread
UTILS = StringUtils.o TCPSocket.o

all: app_servidor app_cliente

clean:
	rm -rf *.o app_servidor app_cliente

# Cliente Stuff
app_cliente: app_cliente.o Cliente.o ${UTILS}
	${CPP} ${FLAGS} -o app_cliente app_cliente.o Cliente.o ${UTILS}

Cliente.o: Cliente.cpp Cliente.hpp
	${CPP} -c Cliente.cpp

app_cliente.o: app_cliente.cpp
	${CPP} -c app_cliente.cpp

# Servidor stuff
app_servidor: app_servidor.o Servidor.o ${UTILS}
	${CPP} ${FLAGS} -o app_servidor app_servidor.o Servidor.o ${UTILS}

Servidor.o: Servidor.cpp Servidor.hpp
	${CPP} -c Servidor.cpp

app_servidor.o: app_servidor.cpp
	${CPP} -c app_servidor.cpp

# Utils
StringUtils.o: StringUtils.cpp StringUtils.hpp
	${CPP} -c StringUtils.cpp

TCPSocket.o: TCPSocket.cpp TCPSocket.hpp
	${CPP} -c TCPSocket.cpp

