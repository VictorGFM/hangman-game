#include "common.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

void validateInputParameters(argc);
int initializeSocket(char *argv[], struct sockaddr_storage *storage);
void connectToServer(int sckt, struct sockaddr_storage *storage);

void printWord(char word[], int wordLength);
void initializeWord(char word[], int wordLength);

int main(int argc, char *argv[]) {
	struct sockaddr_storage storage;

	validateInputParameters(argc);

	int sckt = initializeSocket(argv, &storage);

	connectToServer(sckt, storage)

	char buffer[BUFFER_SIZE];
	memset(buffer, 0, BUFFER_SIZE);
	size_t count = recv(sckt, buffer, BUFFER_SIZE, 0);
	int messageType = (int) buffer[0];
	int wordLength = (int) buffer[1];
	char word[wordLength];
	if(messageType == START_GAME_TYPE) {
		initializeWord(word, wordLength);
		printf("The word has %d letters!\n", wordLength);
		printWord(word, wordLength);
	} else {
		logExit("Error receiving start game message.");
	}
    
    while(messageType != 4) {
        memset(buffer, 0, BUFFER_SIZE);
        printf("Guess a letter: ");
        char letter = getchar();
		getchar(); //Get newline (\n) char to avoid problem on next read.
        buffer[0] = GUESS_SEND_TYPE;
        buffer[1] = letter;
        count = send(sckt, buffer, 2, 0);
        if (count != 2) {
            logExit("Error sending message.");
        }

        memset(buffer, 0, BUFFER_SIZE);
        size_t count = recv(sckt, buffer, BUFFER_SIZE, 0);
        if(count == 0) {
			logExit("Error receiving message. (Invalid format)");
		}
		messageType = (int) buffer[0];
        if(messageType == GUESS_ANSWER_TYPE) {
			int occurrenceNumber = buffer[1];
			for(int i=0; i<occurrenceNumber; i++) {
				int letterPosition = (int) buffer[2+i];
				word[letterPosition] = letter;
			}
        } else if(messageType == END_GAME_TYPE) {
            for(int i=0; i<wordLength; i++) {
				if(word[i] == '_') {
					word[i] = letter;
				}
            }
        } else {
			logExit("Error receiving message. (Invalid message type)");
		}

		printWord(word, wordLength);
    }

	close(sckt);

	printf("CONGRATULATIONS, YOU WON!!!🎉\n");

	exit(EXIT_SUCCESS);
}

void validateInputParameters(argc) {
	if (argc < 3) {
		logExit("Invalid Arguments. Inform a server address and port.\nExample: 127.0.0.1 5151\n");
	}
}

int initializeSocket(char *argv[], struct sockaddr_storage *storage) {
	if (addrparse(argv[1], argv[2], storage) != 0) {
		logExit("Error parsing address.");
	}
	int sckt = socket(storage.ss_family, SOCK_STREAM, 0);
	if (sckt == -1) {
		logExit("Error initializing socket.");
	}
	return sckt;
}

void connectToServer(int sckt, struct sockaddr_storage *storage) {
	struct sockaddr *addr = (struct sockaddr *)(&storage);
	if (connect(sckt, addr, sizeof(storage)) != 0) {
		logExit("Error connecting to server.");
	}
}

void initializeWord(char word[], int wordLength) {
	for(int i=0; i<wordLength; i++) {
		word[i] = '_';
	}
}

void printWord(char word[], int wordLength) {
	printf("|¬\n| ");
	for(int i=0; i<wordLength; i++) {
		printf("%c ", word[i]);
	}
	printf("\n");
}