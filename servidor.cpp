#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h> // inet functions
#include <unistd.h> //fork, close

#define MAX_MSG 129
#define PORT 8080

#define MAX_CLIENTS 2

// https://pubs.opengroup.org/onlinepubs/7908799/xns/syssocket.h.html
// https://www.cs.dartmouth.edu/~campbell/cs60/socketprogramming.html

class Servidor {
    private:
        int serverFD;
        struct sockaddr_in server_address;
        struct sockaddr_in client_address;
        char buffer[MAX_MSG];

    public:
        Servidor() {
            serverFD = socket(AF_INET, SOCK_STREAM, 0);

            server_address.sin_family = AF_INET;
            server_address.sin_addr.s_addr = htonl(INADDR_ANY);
            server_address.sin_port = htons(PORT);

            bind(serverFD, (struct sockaddr*) &server_address, sizeof(server_address));

            listen(serverFD, MAX_CLIENTS);

        }

        void start() {
            socklen_t client_len;
            int connFD, n;
            pid_t childPID;

            printf("Server Started\n");
            while(true) {
                /*client_len = sizeof(client_address);
                connFD = accept(serverFD, (struct sockaddr*) &client_address, &client_len);
                printf("Request received\n");

                while( (n = recv(connFD, buffer, MAX_MSG, 0)) > 0 ) {
                    printf("String received from and resent to client: ");
                    puts(buffer);
                    send(connFD, buffer, n, 0);
                    memset(buffer, 0, sizeof(buffer));
                }

                if (n < 0 ) {
                    perror("Read error");
                    exit(1);
                }
                close(connFD);*/


                client_len = sizeof(client_address);
                printf("Waiting for client to connect...\n");
                connFD = accept(serverFD, (struct sockaddr *) &client_address, &client_len);
                printf("%s\n","Received request...");

                if (fork() == 0) {
                    printf ("Child created for dealing with client requests\n");

                    //close listening socket to stop bugs
                    close(serverFD);

                    while ( (n = recv(connFD, buffer, MAX_MSG, 0)) > 0)  { // Read the message and save in the buffer
                        printf("String received from and resent to the client: ");
                        puts(buffer);
                        send(connFD, buffer, n, 0);
                        memset(buffer, 0, sizeof(buffer));
                    }
                    close(connFD);
                    if (n < 0)
                        printf("%s\n", "Read error");
                    exit(0);
                }
            }
            
            close(serverFD);
        }

        void info() {
            char str[INET_ADDRSTRLEN];
            struct in_addr ipAddr = (&server_address)->sin_addr;
            inet_ntop(AF_INET, &ipAddr, str, INET_ADDRSTRLEN);
            printf("Server running on %s\n", str);
        }
};

int main(int argc, char** argv) {
    Servidor s;
    s.info();
    s.start();

    return 1;
}