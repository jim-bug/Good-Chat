/*
 * Autori: Gianluca Pepe e Ignazio Leonardo Calogero Sperandeo.
 * Data: 04/05/2024
 * Consegna: Realizzare una chat in C che presenti una CLI. La chat deve permettere il dialogo tra due terminali nella stessa LAN e in LAN diverse.
 * Link al repo: https://github.com/jim-bug/Good-Chat
 * Riferimenti alla parte dell'ingegnieria del software: 
 * Nome progetto: Good-Chat
*/

#ifndef CHAT_H
#define CHAT_H

#define MAX_LENGTH_MSG 1024
#define SERVER_PORT 4870        // non modificabile da parte di chi usa la chat.

/*
* Variabili dichiarate come extern perchè ho la necessità di utilizzarne in due sorgenti diversi, fungono da variabili globali per due sorgenti.
* In uno dei due sorgenti dovranno essere dichiarate.
*/
extern WINDOW* input_window;
extern WINDOW* output_window;
extern WINDOW* write_window;
extern FILE* log_file;
extern int start_y;
extern int start_x;
extern pthread_t receive_thread;
extern pthread_t write_thread;

void closing_sequence();        // sequenza di chiusura di finestre, file
void print_stack_trace();       // funzione che manda a video un breve help su come usare la chat.
void write_log(char[], int);    // funzione che scrive sul file di log un messaggio, se riceve 1 come status_code è un messaggio di errore
void create_window(WINDOW**, int, int, int, int);   // funzione che crea una finestra ncurses
void* get_message_from_host(void*);                 // funzione che in base a un file descriptor ottiene i messaggi, in questo caso ottiene i messaggi del client o del server
void* send_message_to_host(void*);                  // funzione che in base a un file descriptor invia un messaggio alla volta al client/sever.
void* listen_threads(void*);

#endif
