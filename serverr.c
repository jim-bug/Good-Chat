#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>
#include <netdb.h>
#include <ncurses.h>
#include <arpa/inet.h>


// pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; // Inizializzazione del mutex
WINDOW* input_window;
WINDOW* output_window;

void create_window(WINDOW** new_win, int width, int height, int x, int y){
	*new_win = newwin(height, width, y, x);
        refresh();
        box(*new_win, '|', '|');
        wrefresh(*new_win);
}

void* get_message_from_host(void* arg) {
    int sockfd = *((int*)arg);
    int row = 1;
    char buf[300]; // Buffer per i dati
    while (1) {
        ssize_t bytes_read = read(sockfd, buf, sizeof(buf));
        if (bytes_read < 0) {
            perror("Errore nella lettura dal sockettttttt");
            break;
        } else if (bytes_read == 0) {
            break;
        }
        buf[bytes_read] = '\0';
//        snprintf(buf, 300, "Client: %s", buf);
        mvwprintw(output_window, row, 1, "Client> %s", buf);
        wrefresh(output_window);
        row ++;
    }

    return NULL;
}

void* send_message_to_host(void* arg) {
    int sockfd = *((int*)arg);
    int row = 1;
    char buf[150];

    while (1) {
        mvwprintw(input_window, row, 1, "Me>");
        mvwgetstr(input_window, row, 4, buf);
        buf[strcspn(buf, "\n")] = '\0';
        wrefresh(input_window);
        ssize_t bytes_written = write(sockfd, buf, strlen(buf));
        if (bytes_written < 0) {
            perror("Errore nella scrittura sul socket 2");
            break;
        }

        row ++;
    }

    return NULL;
}

int main(int argc, char* argv[]) {
    initscr();
    int y, x;
    getmaxyx(stdscr, y, x);
    create_window(&input_window, x/2, y, 0, 0);
    create_window(&output_window, x/2, y, x/2, 0);
    pthread_t receive_thread;
    pthread_t write_thread;

    if ((strcmp(argv[1], "-s") != 0 && strcmp(argv[1], "-c"))) {
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
            endwin();
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
            endwin();
            exit(EXIT_FAILURE);
        }

        // Mette il socket del server in ascolto di connessioni
        if (listen(server_sock, 1) < 0) {
            perror("Errore nella messa in ascolto del socket del server");
            endwin();
            exit(EXIT_FAILURE);
        }

        // Accetta la connessione in arrivo
        client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &client_len);
        if (client_sock < 0) {
            perror("Errore nell'accettazione della connessione");
            endwin();
            exit(EXIT_FAILURE);
        }
        

        // Creazione dei thread per la ricezione e l'invio dei messaggi
        pthread_create(&receive_thread, NULL, get_message_from_host, &client_sock);
        pthread_create(&write_thread, NULL, send_message_to_host, &client_sock);
        pthread_join(receive_thread, NULL);
        pthread_join(write_thread, NULL);
        wrefresh(input_window);
    } else if (strcmp(argv[1], "-c") == 0) {
        int client_sock;
        struct sockaddr_in server_addr;
        struct hostent* hp;

        // Creazione del socket del client
        client_sock = socket(AF_INET, SOCK_STREAM, 0);
        if (client_sock < 0) {
            perror("Errore nella creazione del socket del client");
            endwin();
            exit(EXIT_FAILURE);
        }

        // Risoluzione dell'indirizzo IP del server
        

        // Inizializzazione dell'indirizzo del server
        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        // memcpy(&server_addr.sin_addr, hp->h_addr, hp->h_length);
        server_addr.sin_port = htons(8085);
        inet_pton(AF_INET, argv[2], &server_addr.sin_addr);

        // Connessione al server
        if (connect(client_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            perror("Errore durante la connessione al server");
            endwin();
            exit(EXIT_FAILURE);
        }

        // Creazione dei thread per la ricezione e l'invio dei messaggi
        pthread_create(&receive_thread, NULL, get_message_from_host, &client_sock);
        pthread_create(&write_thread, NULL, send_message_to_host, &client_sock);
        pthread_join(receive_thread, NULL);
        pthread_join(write_thread, NULL);

    }
    wrefresh(input_window);
    wrefresh(output_window);
    delwin(input_window);
    delwin(output_window);
    endwin();
    return 0;
}
