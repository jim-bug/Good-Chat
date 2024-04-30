#ifndef chat
#define chat

#define SERVER_PORT 8085
#define CLIENT_PORT 8000


void* get_message_from_host(int);	// funzione che leggerà tutto quello che il server invierà e lo manderà a video
void* send_message_to_host(int);	// funzione che prenderà in input un messaggio e lo invierà all'host.

#endif
