#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>
#include <netdb.h>

void* get_message_from_host(void* arg) {
    int sockfd = *((int*)arg);
    char buf[150]; // Buffer per i dati

    while (1) {
//        pthread_mutex_lock(&mutex); // Blocco mutex prima della lettura
        ssize_t bytes_read = read(sockfd, buf, sizeof(buf));
//        pthread_mutex_unlock(&mutex); // Rilascio mutex dopo la lettura

        if (bytes_read < 0) {
            perror("Errore nella lettura dal socket");
            break;
        } else if (bytes_read == 0) {
            // Il socket è stato chiuso
            close(sockfd);
            break;
        }
        buf[bytes_read] = '\0';
        printf("Client: %s\n", buf);
    }

    return NULL;
}

// Funzione per inviare messaggi al client
void* send_message_to_host(void* arg) {
    int sockfd = *((int*)arg);
    char buf[150]; // Buffer per i dati

    while (1) {
        printf("Inserisci il messaggio da inviare al client: ");
        fgets(buf, sizeof(buf), stdin);
//        printf("Input preso")
        buf[strcspn(buf, "\n")] = '\0';  // Rimuovi il carattere newline
//        printf("Stringa modificata")
//        pthread_mutex_lock(&mutex); // Blocco mutex prima della scrittura
        ssize_t bytes_written = write(sockfd, buf, strlen(buf));
//        printf("Stringa inviata");
//        pthread_mutex_unlock(&mutex); // Rilascio mutex dopo la scrittura

}

    return NULL;
}


int main(int argc, char* argv[]) {
    pthread_t receive_thread;
    pthread_t write_thread;
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
	    server.sin_port = htons(8085);

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
		if(pthread_create(&receive_thread, NULL, get_message_from_host, &server_sock) != 0) {
        		perror("Errore durante la creazione di receive_thread");
        		exit(EXIT_FAILURE);
    		}

    		if(pthread_create(&write_thread, NULL, send_message_to_host, &server_sock) != 0) {
        		perror("Errore durante la creazione di write_thread");
        		exit(EXIT_FAILURE);
    		}

    		// Attesa del completamento dei thread
    		if(pthread_join(receive_thread, NULL) != 0) {
        		perror("Errore durante l'attesa di receive_thread");
        		exit(EXIT_FAILURE);
    		}

		if(pthread_join(write_thread, NULL) != 0) {
        		perror("Errore durante l'attesa di write_thread");
        		exit(EXIT_FAILURE);
    		}
    }
    else if(strcmp(argv[1], "-c") == 0 && argc == 4){	// specifico uguale a 4 perchè ho bisogno dell'ip e del numero di porta del server.
		    int client_socket;
		    struct sockaddr_in server;
		    struct hostent *hp;
		    client_socket = socket(AF_INET, SOCK_STREAM, 0);
		    if (client_socket < 0) {
		        perror("Socket -> Errore");
		        exit(-1);
		    }

		    server.sin_family = AF_INET;
		    server.sin_port = htons(8085);
 		    hp = gethostbyname("localhost");
		    if (hp == NULL) { // Controllo corretto sull'esito di gethostbyname
 		       perror("Errore localhost");
		       exit(-1);
 		   }

		   memcpy(&server.sin_addr, hp->h_addr, hp->h_length);         // assegno a sin_addr l'indirizzo del server trovato con gethostbyname

		   if (connect(client_socket, (struct sockaddr *)&server, sizeof(server)) < 0) {
		        perror("Errore durante la connessione");
		        exit(-1);
		   }
		   if(pthread_create(&receive_thread, NULL, get_message_from_host, &client_socket) != 0) {
                        perror("Errore durante la creazione di receive_thread");
                        exit(EXIT_FAILURE);
                }

                 if(pthread_create(&write_thread, NULL, send_message_to_host, &client_socket) != 0) {
                        perror("Errore durante la creazione di write_thread");
                        exit(EXIT_FAILURE);
                 }

                // Attesa del completamento dei thread
                 if(pthread_join(receive_thread, NULL) != 0) {
                        perror("Errore durante l'attesa di receive_thread");
                        exit(EXIT_FAILURE);
                }

                if(pthread_join(write_thread, NULL) != 0) {
                        perror("Errore durante l'attesa di write_thread");
                        exit(EXIT_FAILURE);
                }

    }
    else{
	printf("Operazione non valida o non specificata!");
	exit(-1);
    }

    return 0;
}
