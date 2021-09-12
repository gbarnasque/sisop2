#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>

#define MAX_MSG 129
#define PORT 8080

class Cliente {
    private:
        int sockFD;
        struct sockaddr_in serverAddr;
        char sendLine[MAX_MSG], receiveLine[MAX_MSG];

    public:
        Cliente(char* serverIp, char* serverPort) {
            if((sockFD = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
                perror("Problem creating socket");
                exit(2);
            }

            memset(&serverAddr, 0, sizeof(serverAddr));
            serverAddr.sin_family = AF_INET;
            serverAddr.sin_addr.s_addr = inet_addr(serverIp);
            serverAddr.sin_port = htons(atoi(serverPort));

            printf("Trying to connect to server\n");
            if (connect(sockFD, (struct sockaddr *) &serverAddr, sizeof(serverAddr))<0) {
                perror("Problem in connecting to the server");
                exit(3);
            }
            printf("Connected to server\n");
        }

        void sendMessage() {
            printf("Waiting for client input\n");
            while(fgets(sendLine, MAX_MSG, stdin) != NULL) {
                
                send(sockFD, sendLine, strlen(sendLine), 0);

                if(recv(sockFD, receiveLine, MAX_MSG, 0) == 0) {
                    //error: server terminated prematurely
                    perror("The server terminated prematurely");
                    exit(4);
                }
                printf("String received from the server: \n");
                fputs(receiveLine, stdout);

                memset(sendLine, 0, sizeof(sendLine));
                memset(receiveLine, 0, sizeof(receiveLine));
            }
        }
};


int main(int argc, char** argv) {
    char* ip = argv[2];
    char* port = argv[3]; 
    Cliente c(ip, port);
    c.sendMessage();

    return 1;
}