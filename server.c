#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/types.h>

#define IP_VERSION "v4"
#define WORD "MESSAGE"

void validateInputParameters(int argc);
int initializeSocket(char *argv[], struct sockaddr_storage *storage);
void serverBind(int serverSocket, struct sockaddr_storage *storage);
void serverListen(int serverSocket);
int acceptClientConnection(int serverSocket);
void sendAckToClient(int clientSocket);
void initializeGuessedWord(char guessedWord[], int wordLength);
void receiveGuessFromClient(int clientSocket, char buffer[], int BUFFER_SIZE);
int checkLetterGuess(char buffer[], int BUFFER_SIZE, char guessedWord[]);
void sendAnswerToClient(char buffer[], int BUFFER_SIZE, char guessedWord[], int occurrenceNumber, int clientSocket);

int main(int argc, char *argv[]) {
    struct sockaddr_storage storage;

    validateInputParameters(argc);

    int serverSocket = initializeSocket(argv, &storage);

    serverBind(serverSocket, &storage);

    serverListen(serverSocket);

    while (1) {
        int clientSocket = acceptClientConnection(serverSocket);

        sendAckToClient(clientSocket);

        char guessedWord[strlen(WORD)];
        initializeGuessedWord(guessedWord, strlen(WORD));

        while(strcmp(guessedWord, WORD) != 0) {
            const int BUFFER_SIZE = 2;
            char buffer[BUFFER_SIZE];
            memset(buffer, 0, BUFFER_SIZE);
            receiveGuessFromClient(clientSocket, buffer, BUFFER_SIZE);

            int messageType = (int) buffer[0];
            if(messageType == GUESS_SEND_TYPE) {
                int occurrenceNumber = checkLetterGuess(buffer, BUFFER_SIZE, guessedWord);
                sendAnswerToClient(buffer, BUFFER_SIZE, guessedWord, occurrenceNumber, clientSocket);
            } else {
                logExit("Error receiving message. (Invalid message type)");
            }
        }
        
        close(clientSocket);
    }

    exit(EXIT_SUCCESS);
}

void validateInputParameters(int argc) {
    if (argc < 2) {
		logExit("Invalid Arguments. Inform a server port.\nExample: 5151\n");
    }
}

int initializeSocket(char *argv[], struct sockaddr_storage *storage) {
    if(initializeServerAddress(IP_VERSION, argv[1], storage) != 0) {
        logExit("Error initializing address.");
    }
    int serverSocket = socket(storage->ss_family, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        logExit("Error initializing socket.");
    }
    int enable = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) != 0) {
        logExit("Error setting socket options.");
    }
    return serverSocket;
}

void serverBind(int serverSocket, struct sockaddr_storage *storage) {
    struct sockaddr *addr = (struct sockaddr *)(storage);
    if (bind(serverSocket, addr, sizeof(*storage)) != 0) {
        logExit("Error on bind.");
    }
}

void serverListen(int serverSocket) {
    if (listen(serverSocket, 1) != 0) {
        logExit("Error on listen.");
    }
}

int acceptClientConnection(int serverSocket) {
    struct sockaddr_storage clientStorage;
    struct sockaddr *clientAddress = (struct sockaddr *)(&clientStorage);
    socklen_t clientAddressLength = sizeof(clientStorage);

    int clientSocket = accept(serverSocket, clientAddress, &clientAddressLength);
    if (clientSocket == -1) {
        logExit("Error accepting client connection.");
    }
    return clientSocket;
}

void sendAckToClient(int clientSocket) {
    const int BUFFER_SIZE = 2;
    char buffer[BUFFER_SIZE];
    buffer[0] = START_GAME_TYPE;
    buffer[1]= strlen(WORD);
    size_t count = send(clientSocket, buffer, BUFFER_SIZE, 0);
    if (count != 2) {
        logExit("Error sending message.");
    }
}

void initializeGuessedWord(char guessedWord[], int wordLength) {
    for(int i=0; i<wordLength; i++) {
        guessedWord[i] = '_';
    }
}

void receiveGuessFromClient(int clientSocket, char buffer[], int BUFFER_SIZE) {
    size_t count = recv(clientSocket, buffer, BUFFER_SIZE, 0);
    if(count == 0) {
        logExit("Error receiving message. (Invalid format)");
    }
}

int checkLetterGuess(char buffer[], int  BUFFER_SIZE, char guessedWord[]) {
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
    return occurrenceNumber;
}

void sendAnswerToClient(char buffer[], int BUFFER_SIZE, char guessedWord[], int occurrenceNumber, 
                        int clientSocket) {
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
        size_t count = send(clientSocket, buffer, 1, 0);
        if (count != 1) {
            logExit("Error sending message.");
        }
    }
}