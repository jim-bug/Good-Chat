
/*
 * Autori: Gianluca Pepe e Ignazio Leonardo Calogero Sperandeo.
 * Data: 04/05/2024
 * Consegna: Realizzare una chat in C che presenti una CLI. La chat deve permettere il dialogo tra due terminali nella stessa LAN e in LAN diverse.
 * Link al repo: https://github.com/jim-bug/Good-Chat
 * Riferimenti alla parte dell'ingegnieria del software: 
 * Nome progetto: Good-Chat
*/

#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "chat.h"



pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int window_rows_sharing = 1;
char flag_state_close = 'n';


void closing_sequence(){
        delwin(input_window);
        delwin(output_window);
        delwin(write_window);
        endwin();
        fclose(log_file);
}

void print_stack_trace(){
        closing_sequence();
        printf("\tHelp good-chat!\nUsage: -s | -c ip port\n\t1) -s: Opzione server\n\t2) -c: Opzione client, speficare anche l'ip e il numero di porta(compreso tra 1024 e 49515) del server\n");
        exit(EXIT_FAILURE);
}

void write_log(char log_message[], int state_error){
        if(state_error == 1){       // caso di errore.
            fprintf(log_file, "%s\n", log_message);
            closing_sequence();
            exit(EXIT_FAILURE);
        }
        fprintf(log_file, "%s\n", log_message);
}

// Funzione che crea una finestra con una box
void create_window(WINDOW** new_win, int row, int col, int begin_y, int begin_x){
	    *new_win = newwin(row, col, begin_y, begin_x);
        refresh();
        // box(*new_win, '|', '|');     // piccolo debug per le finestre
        wrefresh(*new_win);
}

void* get_message_from_host(void* arg) {        // funzione che riceve qualcosa dal server/client
    int sockfd = *((int*)arg);
    char buf[MAX_LENGTH_MSG]; // Buffer per i dati
    ssize_t bytes_read;
    do {
        bytes_read = recv(sockfd, buf, sizeof(buf), 0);
        buf[bytes_read] = '\0';
        mvwprintw(output_window, window_rows_sharing, 1, "Client> %s", buf);
    

        pthread_mutex_lock(&mutex);
        // sezione critica, inizio
        if(window_rows_sharing >= (start_y-4)){
        wclear(input_window);
        wclear(output_window);
        window_rows_sharing = 1;      // reimposto il puntatore a 1, così i messaggi li visualizzo nella finestra pulita.    
        }
        else{
            window_rows_sharing ++;
        }
        // sezione critica, fine
        pthread_mutex_unlock(&mutex);
        wrefresh(output_window);

    } while(strcmp(buf, "exit") != 0 || bytes_read <= 0);
    flag_state_close = 'y';
    return NULL;
}

void* send_message_to_host(void* arg) {     // funzione che invia qualcosa al server/client
    int sockfd = *((int*)arg);
    char buf[MAX_LENGTH_MSG];
    ssize_t bytes_written;
    do {
        mvwprintw(write_window, 1, 1, "Me> ");      // chiedo all'utente cosa vuole mandare all'altro host su una terza finestra.
        mvwgetstr(write_window, 1, 4, buf);
        mvwprintw(input_window, window_rows_sharing, 1, "Me> %s", buf);       // mando a video sulla finestra di input ciò che ho inviato
        wclear(write_window);

        bytes_written = write(sockfd, buf, strlen(buf)+1);
        fprintf(log_file, "%s", "Bloccante?");
        pthread_mutex_lock(&mutex);
        // sezione critica, inizio
        if(window_rows_sharing >= (start_y-4)){
            wclear(input_window);
            wclear(output_window);
            window_rows_sharing = 1;    
        }
        else{
            window_rows_sharing ++;
        }
        // sezione critica, fine
        pthread_mutex_unlock(&mutex);

        wrefresh(input_window);
        wrefresh(write_window);
    } while(strcmp(buf, "exit") != 0 || bytes_written <= 0);
    flag_state_close = 'y';
    return NULL;
}

void* listen_threads(void* arg){
    while(flag_state_close != 'y');
    pthread_cancel(receive_thread);
    pthread_cancel(write_thread);
    closing_sequence();
    exit(EXIT_SUCCESS);
}
