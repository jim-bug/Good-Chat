#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>
#include <netdb.h>
#include <ncurses.h>


pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; // Inizializzazione del mutex
WINDOW *win;

int y_prompt = 2;
int x_prompt = 70;
int i = -1;

void* get_message_from_host(void* arg) {
    int sockfd = *((int*)arg);
    char buf[150]; // Buffer per i dati
    while (1) {
        ssize_t bytes_read = read(sockfd, buf, sizeof(buf));
        if (bytes_read < 0) {
            perror("Errore nella lettura dal sockettttttt");
            break;
        } else if (bytes_read == 0) {
//            close(sockfd);
            break;
        }
        buf[bytes_read] = '\0';
       
        pthread_mutex_lock(&mutex);
         i++;
        pthread_mutex_unlock(&mutex);
        mvprintw(i, 1, "Client: %s", buf);
       refresh();
    }

    return NULL;
}

void* send_message_to_host(void* arg) {
    int sockfd = *((int*)arg);
    char buf[150];

    while (1) {
        buf[strcspn(buf, "\n")] = '\0';
        ssize_t bytes_written = write(sockfd, buf, strlen(buf));
        if (bytes_written < 0) {
            perror("Errore nella scrittura sul socket 2");
            break;
        }
        pthread_mutex_lock(&mutex);
	i++;
        pthread_mutex_unlock(&mutex);
        mvprintw(i, 1, "Inserisci il messaggio da inviare al client: ");
        mvscanw(i, 70, "%s", buf);
        refresh();
    }

    return NULL;
}

int main(int argc, char* argv[]) {
    initscr();
    win = newwin(24, 80, 0, 0);

    pthread_t receive_thread;
    pthread_t write_thread;

    if (argc != 2 || (strcmp(argv[1], "-s") != 0 && strcmp(argv[1], "-c") != 0)) {
        fprintf(stderr, "Utilizzo: %s -s|-c\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if (strcmp(argv[1], "-s") == 0) {
        int server_sock, client_sock;
        struct sockaddr_in server_addr, client_addr;
        socklen_t client_len = sizeof(client_addr);

        // Creazione del socket del server
        server_sock = socket(AF_INET, SOCK_STREAM, 0);
        if (server_sock < 0) {
            perror("Errore nella creazione del socket del server");
            exit(EXIT_FAILURE);
        }

        // Inizializzazione dell'indirizzo del server
        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = INADDR_ANY;
        server_addr.sin_port = htons(8085);

        // Binding del socket del server all'indirizzo locale
        if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            perror("Errore nel binding del socket del server");
            exit(EXIT_FAILURE);
        }

        // Mette il socket del server in ascolto di connessioni
        if (listen(server_sock, 1) < 0) {
            perror("Errore nella messa in ascolto del socket del server");
            exit(EXIT_FAILURE);
        }

        // Accetta la connessione in arrivo
        client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &client_len);
        if (client_sock < 0) {
            perror("Errore nell'accettazione della connessione");
            exit(EXIT_FAILURE);
        }

        // Creazione dei thread per la ricezione e l'invio dei messaggi
        if (pthread_create(&receive_thread, NULL, get_message_from_host, &client_sock) != 0) {
            perror("Errore durante la creazione del thread di ricezione");
            exit(EXIT_FAILURE);
        }

        if (pthread_create(&write_thread, NULL, send_message_to_host, &client_sock) != 0) {
            perror("Errore durante la creazione del thread di invio");
            exit(EXIT_FAILURE);
        }

        // Attesa del completamento dei thread
        if (pthread_join(receive_thread, NULL) != 0) {
            perror("Errore durante l'attesa del thread di ricezione");
            exit(EXIT_FAILURE);
        }

        if (pthread_join(write_thread, NULL) != 0) {
            perror("Errore durante l'attesa del thread di invio");
            exit(EXIT_FAILURE);
        }
    } else if (strcmp(argv[1], "-c") == 0) {
        int client_sock;
        struct sockaddr_in server_addr;
        struct hostent* hp;

        // Creazione del socket del client
        client_sock = socket(AF_INET, SOCK_STREAM, 0);
        if (client_sock < 0) {
            perror("Errore nella creazione del socket del client");
            exit(EXIT_FAILURE);
        }

        // Risoluzione dell'indirizzo IP del server
        hp = gethostbyname("localhost");
        if (hp == NULL) {
            fprintf(stderr, "Errore nella risoluzione dell'host\n");
            exit(EXIT_FAILURE);
        }

        // Inizializzazione dell'indirizzo del server
        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        memcpy(&server_addr.sin_addr, hp->h_addr, hp->h_length);
        server_addr.sin_port = htons(8085);

        // Connessione al server
        if (connect(client_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            perror("Errore durante la connessione al server");
            exit(EXIT_FAILURE);
        }

        // Creazione dei thread per la ricezione e l'invio dei messaggi
        if (pthread_create(&receive_thread, NULL, get_message_from_host, &client_sock) != 0) {
            perror("Errore durante la creazione del thread di ricezione");
            exit(EXIT_FAILURE);
        }

        if (pthread_create(&write_thread, NULL, send_message_to_host, &client_sock) != 0) {
            perror("Errore durante la creazione del thread di invio");
            exit(EXIT_FAILURE);
        }

        // Attesa del completamento dei thread
        if (pthread_join(receive_thread, NULL) != 0) {
            perror("Errore durante l'attesa del thread di ricezione");
            exit(EXIT_FAILURE);
        }

        if (pthread_join(write_thread, NULL) != 0) {
            perror("Errore durante l'attesa del thread di invio");
            exit(EXIT_FAILURE);
        }
    }
    endwin();
    return 0;
}
