#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <unistd.h>
#include "chat.h"

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
void* get_message_from_host(void* arg){
	int* socket = (int*)arg;
	while(1){
                char message[150];
		pthread_mutex_lock(&mutex);
        	ssize_t bytes_read = read(*socket, message, 150);
		pthread_mutex_unlock(&mutex);
        	if (bytes_read <= 0) {
            		printf("Connessione chiusa dal client %ld\n", bytes_read);
            		// close(*socket);
            		continue;
        	}
		else if(strcmp(message, "exit") == 0){
			close(*socket);
			break;
		}
                else{
		       message[bytes_read] =  '\0';
                       printf("Client: %s\n", message);

            }

    	}
	return NULL;
}
void* send_message_to_host(void* arg){
	while(1){
		int* socket = (int *)arg;
        	char message[150];
        	printf("Inserisci il messaggio da inviare al client: ");
        	fgets(message, 150, stdin);
        	message[strcspn(message, "\n")] = '\0';
		pthread_mutex_lock(&mutex);
		int write_output = write(*socket, message, 150);
		pthread_mutex_unlock(&mutex);
        	if(write_output < 0){
                	printf("Errore con il messaggio %s", message);
        	}
	}
	return NULL;
}
