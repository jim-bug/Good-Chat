#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <netdb.h>
#include <ncurses.h>
#include <arpa/inet.h>

#define SERVER_PORT 4869
#define MAX_LENGTH_MSG 1024

/*
 * Autori: Gianluca Pepe e Ignazio Leonardo Calogero Sperandeo.
 * Data: 04/05/2024
 * Consegna: Realizzare una chat in C che presenti una CLI. La chat deve permettere il dialogo tra due terminali nella stessa LAN e in LAN diverse.
 * Link al repo: https://github.com/jim-bug/Good-Chat
 * Nome progetto: Good-Chat
*/

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
WINDOW* input_window;
WINDOW* output_window;
WINDOW* win3;
int start_y, start_x;

int row_shared_window = 1;

void print_stack_trace(){
        delwin(input_window);
        delwin(output_window);
        endwin();
        printf("\tHelp good-chat!\nUsage: -s | -c ip port\n\t1) -s: Opzione server\n\t2) -c: Opzione client, speficare anche l'ip e il numero di porta del server\n");
        exit(EXIT_FAILURE);
}

// Funzione che crea una finestra con una box
void create_window(WINDOW** new_win, int row, int col, int begin_y, int begin_x){
	    *new_win = newwin(row, col, begin_y, begin_x);
        refresh();
        // box(*new_win, '|', '|');     // piccolo debug per le finestre
        wrefresh(*new_win);
}

void* get_message_from_host(void* arg) {
    int sockfd = *((int*)arg);
    char buf[MAX_LENGTH_MSG]; // Buffer per i dati
    while (1) {
        ssize_t bytes_read = recv(sockfd, buf, sizeof(buf), 0);
        if (bytes_read < 0) {
            perror("Errore nella lettura dal socket");
            break;
        } 
        else if (bytes_read == 0) {
            break;
        }
        buf[bytes_read] = '\0';
//        snprintf(buf, 300, "Client: %s", buf);
        mvwprintw(output_window, row_shared_window, 1, "Client> %s", buf);
        wrefresh(output_window);

        pthread_mutex_lock(&mutex);
        if(row_shared_window >= (start_y-4)){
            wclear(input_window);
            wclear(output_window);
            row_shared_window = 1;    
        }
        else{
            row_shared_window ++;
        }
        wrefresh(output_window);
        pthread_mutex_unlock(&mutex);
    }

    return NULL;
}

void* send_message_to_host(void* arg) {
    int sockfd = *((int*)arg);
    char buf[MAX_LENGTH_MSG];

    while (1) {
        if(row_shared_window >= (start_y-4)){
            wclear(input_window);
            row_shared_window = 1;    
        }
        mvwprintw(win3, 1, 1, "Me> ");
        mvwgetstr(win3, 1, 4, buf);
        // buf[strcspn(buf, "\n")] = '\0';
        mvwprintw(input_window, row_shared_window, 1, "Me> %s", buf);       // mando a video sulla finestra di input ciò che ho inviato
        wclear(win3);
        ssize_t bytes_written = write(sockfd, buf, strlen(buf)+1);
        if (bytes_written < 0) {
            perror("Errore nella scrittura sul socket 2");
            break;
        }
        wrefresh(input_window);
        wrefresh(win3);

        pthread_mutex_lock(&mutex);
        if(row_shared_window >= (start_y-4)){
            wclear(input_window);
            wclear(output_window);
            row_shared_window = 1;    
        }
        else{
            row_shared_window ++;
        }
        wrefresh(input_window);
        pthread_mutex_unlock(&mutex);
    }

    return NULL;
}

int main(int argc, char* argv[]) {
    pthread_t receive_thread;
    pthread_t write_thread;
    FILE* log = fopen("log.txt", "w");
    initscr(); // Inizializza la finestra ncurses principale

    getmaxyx(stdscr, start_y, start_x); // Ottenere le dimensioni dello schermo
    create_window(&input_window, start_y-4, start_x/2, 0, 0);
    create_window(&output_window, start_y-4, start_x/2, 0, start_x/2);
    create_window(&win3, 4, start_x, start_y-4, 0);


    if (argc < 2) {
        print_stack_trace();
    }
    else if(strcmp(argv[1], "-s") != 0 && strcmp(argv[1], "-c") != 0){
        print_stack_trace();
    }
    else if (strcmp(argv[1], "-s") == 0) {
        int server_sock, client_sock;
        struct sockaddr_in server_addr, client_addr;
        socklen_t client_len = sizeof(client_addr);

        // Creazione del socket del server
        server_sock = socket(AF_INET, SOCK_STREAM, 0);
        if (server_sock < 0) {
            endwin();	// in caso di errore da parte di ogni primitiva cancello l'intera finestra ncurses.
	        fprintf(log, "%s\n", "Creazione mezzo socket del server -> ERRORE");
            // perror("Errore nella creazione del socket del server");
            exit(EXIT_FAILURE);
        }
        fprintf(log, "%s\n", "Creazione mezzo socket del server -> OK");


        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = INADDR_ANY;
        server_addr.sin_port = htons(SERVER_PORT);

        // Binding del mezzo socket del server all'indirizzo locale
        if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            endwin();
            fprintf(log, "%s\n", "Binding del mezzo socket del server -> ERRORE");
            // perror("Errore nel binding del socket del server");
            exit(EXIT_FAILURE);
        }
        
        fprintf(log, "%s\n", "Binding del mezzo socket del server -> OK");
        
        // Metto il mezzo socket del server in ascolto di connessioni, con una coda massima di 1 persona.
        if (listen(server_sock, 1) < 0) {
            endwin();
            fprintf(log, "%s\n", "Listen del mezzo socket del server -> ERRORE");
            // perror("Errore nella messa in ascolto del socket del server");
            exit(EXIT_FAILURE);
        }
        fprintf(log, "%s\n", "Listen del mezzo socket del server -> OK");

        // Metto il mezzo socket del server in grado di accettare connessioni
        client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &client_len);
        if (client_sock < 0) {
            // perror("Errore nell'accettazione della connessione");
            endwin();
            fprintf(log, "%s\n", "Accettazione della connessione -> ERRORE");
            exit(EXIT_FAILURE);
        }
        fprintf(log, "%s\n", "Accettazione della connessione -> OK");
        fclose(log);

        // Creazione dei thread per la ricezione e l'invio dei messaggi
        pthread_create(&receive_thread, NULL, get_message_from_host, &client_sock);
        pthread_create(&write_thread, NULL, send_message_to_host, &client_sock);
        pthread_join(receive_thread, NULL);
        pthread_join(write_thread, NULL);
        close(server_sock);
        wrefresh(input_window);
    } 
    else if (strcmp(argv[1], "-c") == 0 && atoi(argv[3]) >= 1024 && atoi(argv[3]) <= 49151) {
        int client_sock;
        unsigned short port = (unsigned short)atoi(argv[3]);
        struct sockaddr_in server_addr;
        struct hostent* hp;

        // Creazione del socket del client
        client_sock = socket(AF_INET, SOCK_STREAM, 0);
        if (client_sock < 0) {
            // perror("Errore nella creazione del socket del client");
            endwin();
            fprintf(log, "%s\n", "Creazione del mezzo socket del client -> ERRORE");
            exit(EXIT_FAILURE);
        }
        fprintf(log, "%s\n", "Creazione del mezzo socket del client -> OK");
        
        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;

        server_addr.sin_port = htons(port);        // converto in intero la stringa che indica il numero di porta
        inet_pton(AF_INET, argv[2], &server_addr.sin_addr);	// assegno l'ip del server alla quale il client si dovrà connettere.

        // Connessione al server
        if (connect(client_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            fprintf(log, "%s\n", "Connessione al server -> ERRORE");
            // perror("Errore durante la connessione al server");
            endwin();
            exit(EXIT_FAILURE);
        }
        fprintf(log, "%s\n", "Connessione al server -> OK");
        // Creazione dei thread per la ricezione e l'invio dei messaggi
        fclose(log);
        pthread_create(&receive_thread, NULL, get_message_from_host, &client_sock);
        pthread_create(&write_thread, NULL, send_message_to_host, &client_sock);
        pthread_join(receive_thread, NULL);
        pthread_join(write_thread, NULL);
        close(client_sock);
    }
    else{
        print_stack_trace();
    }
    wrefresh(input_window);
    wrefresh(output_window);
    delwin(input_window);
    delwin(output_window);
    endwin();
    return 0;
}