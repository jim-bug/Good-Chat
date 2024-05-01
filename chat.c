#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "chat.h"

// Mutex per garantire l'accesso esclusivo ai socket
pthread_mutex_t socket_mutex = PTHREAD_MUTEX_INITIALIZER;

// Funzione per leggere da un socket
void* get_message_from_host(void* sockfd_ptr) {
    int sockfd = *((int*)sockfd_ptr);
    char buf[150]; // Buffer per i dati

    while (1) {
        pthread_mutex_lock(&socket_mutex); // Blocco mutex prima della lettura
        ssize_t bytes_read = read(sockfd, buf, sizeof(buf));
        pthread_mutex_unlock(&socket_mutex); // Rilascio mutex dopo la lettura

        if (bytes_read < 0) {
//            perror("Errore nella lettura dal socket");
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

// Funzione per scrivere su un socket
void* send_message_to_host(void* sockfd_ptr) {
    int sockfd = *((int*)sockfd_ptr);
    char buf[150]; // Buffer per i dati

    while (1) {
        printf("Inserisci il messaggio da inviare al client: ");
        fgets(buf, sizeof(buf), stdin);

        pthread_mutex_lock(&socket_mutex); // Blocco mutex prima della scrittura
        ssize_t bytes_written = write(sockfd, buf, strlen(buf));
        pthread_mutex_unlock(&socket_mutex); // Rilascio mutex dopo la scrittura

        if (bytes_written < 0) {
            perror("Errore nella scrittura sul socket");
            break;
        }
    }

    return NULL;
}
