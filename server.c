#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/types.h>

#define IP_VERSION "v4"
#define WORD "NASTAFF"
#define ALL_LETTERS_FOUND "*******"

int main(int argc, char *argv[]) {
    struct sockaddr_storage storage;
    if (argc < 2 || server_sockaddr_init(IP_VERSION, argv[1], &storage) != 0) {
		logExit("Invalid Arguments. Inform a server port.\nExample: 5151\n");
    }

    int serverSocket;
    serverSocket = socket(storage.ss_family, SOCK_STREAM, 0);
    if (serverSocket == -1) {
		logExit("Error initializing socket.");
    }
    int enable = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) != 0) {
		logExit("Error setting socket options.");
    }

    struct sockaddr *addr = (struct sockaddr *)(&storage);
    if (bind(serverSocket, addr, sizeof(storage)) != 0) {
		logExit("Error on bind.");
    }

    if (listen(serverSocket, 10) != 0) {
		logExit("Error on listen.");
    }

    while (1) {
        struct sockaddr_storage clientStorage;
        struct sockaddr *clientAddress = (struct sockaddr *)(&clientStorage);
        socklen_t clientAddressLength = sizeof(clientStorage);

        int clientSocket = accept(serverSocket, clientAddress, &clientAddressLength);
        if (clientSocket == -1) {
		    logExit("Error accepting client connection.");
        }

        char buffer[BUFFER_SIZE];
        buffer[0] = START_GAME_TYPE;
        buffer[1]= strlen(WORD);
        size_t count = send(clientSocket, buffer, 2, 0);
        if (count != 2) {
            logExit("Error sending message.");
        }

        char guessedWord[strlen(WORD)];
        for(int i=0; i<strlen(WORD); i++) {
            guessedWord[i] = '_';
        }
        while(strcmp(guessedWord, WORD) != 0) {
            memset(buffer, 0, BUFFER_SIZE);
            count = recv(clientSocket, buffer, BUFFER_SIZE - 1, 0);
            if(count == 0) {
                logExit("Error receiving message. (Invalid format)");
            }
            int messageType = (int) buffer[0];
            if(messageType == GUESS_SEND_TYPE) {
                char letter = buffer[1];
                memset(buffer, 0, BUFFER_SIZE);
                char *letterFound = strchr(WORD, letter);
                int occurrenceNumber = 0;
                while (letterFound!=NULL) {
                    int letterPosition = (letterFound-WORD);
                    buffer[2+occurrenceNumber] = letterPosition;
                    guessedWord[letterPosition] = letter;
                    occurrenceNumber++;
                    letterFound=strchr(letterFound+1,letter);
                }
                if(strcmp(guessedWord, WORD) != 0) {
                    buffer[0] = GUESS_ANSWER_TYPE;
                    buffer[1] = occurrenceNumber;
                    size_t count = send(clientSocket, buffer, 2+occurrenceNumber, 0);
                    if (count != 2+occurrenceNumber) {
                        logExit("Error sending message.");
                    }
                } else {
                    memset(buffer, 0, BUFFER_SIZE);
                    buffer[0] = END_GAME_TYPE;
                    count = send(clientSocket, buffer, 1, 0);
                    if (count != 1) {
                        logExit("Error sending message.");
                    }
                }
            } else {
			    logExit("Error receiving message. (Invalid message type)");
            }
        }
        
        close(clientSocket);
    }

    exit(EXIT_SUCCESS);
}