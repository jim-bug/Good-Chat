#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include "chat.h"

void* send_message_to_host(int socket){
        char message[150];
        fgets(message, 150, stdin);
        message[strcspn(message, "\n")] = '\0';
        if(write(socket, message, 150) < 0){
                printf("Errore con il messaggio %s", message);
        }
}
