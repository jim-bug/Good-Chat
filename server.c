#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>
#include "chat.h"


int main(int argc, char* argv[]) {
    if(strcmp(argv[1], "-s") == 0){
	    int server_sock;
	    struct sockaddr_in server, client;
	    int length = sizeof(client);
	    int complete_socket;
	    server_sock = socket(AF_INET, SOCK_STREAM, 0);              //  l'OS associa al socket un file descriptor con le seguenti caratteristiche.
	    if (server_sock < 0) {
        		perror("Creazione della socket -> Errore");
        		exit(-1);
	    }

	    // Popolazione delle informazioni del mezzo socket dedicato al server, quindi ipv4 + porta e famiglia di indirizzi Ipv4.
	    server.sin_family = AF_INET;
	    server.sin_addr.s_addr = INADDR_ANY;
	    server.sin_port = htons(SERVER_PORT);

	    // associo al file descriptor la struct con tutti gli argomenti in erenti al mezzo socket del server
	    if (bind(server_sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
		        perror("Binding server -> Errore");
	        	exit(-1);
    	    }

	    if (getsockname(server_sock, (struct sockaddr *)&server, &length) < 0) {
		        perror("Errore con getsockname()");
		        exit(-1);
	    }

	    // metto in ascolto con una coda massima di 1, il mezzo socket dedicato al server.
	    if (listen(server_sock, 1) < 0) {
        		perror("Listen -> Errore");
		        exit(-1);
	    }

	    complete_socket = accept(server_sock, (struct sockaddr *)&client, (socklen_t *)&length);
	    if (complete_socket < 0) {
		        perror("Errore accept");
	                exit(-1);
            }

    }
    else if(strcmp(argv[1], "-c") == 0 && argc == 4){	// specifico uguale a 4 perchè ho bisogno dell'ip e del numero di porta del server.
	printf("Ancora da implementare!");
    }
    else{
	printf("Operazione non valida o non specificata!");
	exit(-1);
    }

    return 0;
}
