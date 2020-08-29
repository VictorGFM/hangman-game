#include "common.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

void logExit(const char *message);

#define BUFSZ 1024
#define GUESS_ANSWER_TYPE 3
#define END_GAME_TYPE 4

int main(int argc, char **argv) {
	if (argc < 3) {
		logExit("Invalid Arguments. Inform a server address and port.\nExample: 127.0.0.1 51511\n");
	}

	struct sockaddr_storage storage;
	if (addrparse(argv[1], argv[2], &storage) != 0) {
		logExit("Invalid Arguments. Inform a server address and port.\nExample: 127.0.0.1 51511\n");
	}
	int sckt = socket(storage.ss_family, SOCK_STREAM, 0);
	if (sckt == -1) {
		logExit("Error initializing socket!");
	}

	struct sockaddr *addr = (struct sockaddr *)(&storage);
	if (connect(sckt, addr, sizeof(storage)) != 0) {
		logExit("Error connecting to server.");
	}
	char addrstr[BUFSZ];
	addrtostr(addr, addrstr, BUFSZ);
	printf("Connected to server %s\n", addrstr);

	char buffer[BUFSZ];
	memset(buffer, 0, BUFSZ);
	size_t count = recv(sckt, buffer, BUFSZ, 0);
    printf("The word has %s letters!", buffer[1])
	
    int messageType = 0;
    while(messageType != 4) {
        memset(buffer, 0, BUFSZ);
        printf("Guess a letter: ");
        char letter = getchar();
        buffer[0] = '2';
        buffer[1] = letter;
        buffer[2] = '\0';
        count = send(sckt, buffer, strlen(buffer)+1, 0);
        if (count != strlen(buffer)+1) {
            logExit("Error sending message.");
        }

        memset(buffer, 0, BUFSZ);
        size_t count = recv(sckt, buffer, BUFSZ, 0);
        messageType = buffer[0]
        if(messageType == GUESS_ANSWER_TYPE) {

        } else if(messageType == END_GAME_TYPE) {
            for(int i=0; i<buffer[1]) {

            }
        }
    }
	close(sckt);

	printf("received %zu bytes\n", count);
	puts(buffer);

	exit(EXIT_SUCCESS);
}

void logExit(const char *message) {
	printf("%s", message);
	exit(EXIT_FAILURE);
}