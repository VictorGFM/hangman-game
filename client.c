#include "common.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

void validateInputParameters(int argc);
int initializeSocket(char *argv[], struct sockaddr_storage *storage);
void connectToServer(int sckt, struct sockaddr_storage *storage);
int receiveAckFromServer(int sckt);
void initializeWord(char word[], int wordLength);
void printWord(char word[]);
char readLetter();
void sendGuess(int sckt, char letter);
int receiveAnswerFromServer(int sckt, char word[], char letter);
void setWordLetters(char buffer[], char word[], char letter);

int main(int argc, char *argv[]) {
	struct sockaddr_storage storage;

	validateInputParameters(argc);

	int sckt = initializeSocket(argv, &storage);

	connectToServer(sckt, &storage);

	int wordLength = receiveAckFromServer(sckt);	
	char word[wordLength];
	initializeWord(word, wordLength);
	printWord(word);
    
	int messageType = 0;
	while(messageType != END_GAME_TYPE) {
		char letter = readLetter();
		sendGuess(sckt, letter);
		messageType = receiveAnswerFromServer(sckt, word, letter);
		printWord(word);
	}

	close(sckt);

	printf("CONGRATULATIONS, YOU WON!!!🎉\n");

	exit(EXIT_SUCCESS);
}

void validateInputParameters(int argc) {
	if (argc < 3) {
		logExit("Invalid Arguments. Inform a server address and port.\nExample: 127.0.0.1 5151\n");
	}
}

int initializeSocket(char *argv[], struct sockaddr_storage *storage) {
	if (addressParse(argv[1], argv[2], storage) != 0) {
		logExit("Error parsing address.");
	}
	int sckt = socket(storage->ss_family, SOCK_STREAM, 0);
	if (sckt == -1) {
		logExit("Error initializing socket.");
	}
	return sckt;
}

void connectToServer(int sckt, struct sockaddr_storage *storage) {
	struct sockaddr *addr = (struct sockaddr *)(storage);
	if (connect(sckt, addr, sizeof(*storage)) != 0) {
		logExit("Error connecting to server.");
	}
}

int receiveAckFromServer(int sckt) {
	const int BUFFER_SIZE = 2;
	char buffer[BUFFER_SIZE];
	memset(buffer, 0, BUFFER_SIZE);
	size_t count = recv(sckt, buffer, BUFFER_SIZE, 0);
	if(count == 0) {
		logExit("Error receiving message. (Invalid format)");
	}
	int messageType = (int) buffer[0];
	int wordLength = (int) buffer[1];
	if(messageType == START_GAME_TYPE) {
		printf("The word has %d letters!\n", wordLength);
	} else {
		logExit("Error receiving start game message.");
	}
	return wordLength;
}

void initializeWord(char word[], int wordLength) {
	for(int i=0; i<wordLength; i++) {
		word[i] = '_';
	}
}

void printWord(char word[]) {
	printf("|¬\n| ");
	for(int i=0; i<strlen(word); i++) {
		printf("%c ", word[i]);
	}
	printf("\n");
}

char readLetter() {
	printf("Guess a letter: ");
	char letter = getchar();
	getchar(); //Get newline (\n) char to avoid problem on next read.
	return letter;
}

void sendGuess(int sckt, char letter) {
	const int BUFFER_SIZE = 2;
	char buffer[BUFFER_SIZE];
	memset(buffer, 0, BUFFER_SIZE);
	buffer[0] = GUESS_SEND_TYPE;
	buffer[1] = letter;
	size_t count = send(sckt, buffer, 2, 0);
	if (count != 2) {
		logExit("Error sending message.");
	}
}

int receiveAnswerFromServer(int sckt, char word[], char letter) {
	const int BUFFER_SIZE = 2+strlen(word);
	char buffer[BUFFER_SIZE];
	memset(buffer, 0, BUFFER_SIZE);
	size_t count = recv(sckt, buffer, BUFFER_SIZE, 0);
	if(count == 0) {
		logExit("Error receiving message. (Invalid format)");
	}
	int messageType = (int) buffer[0];
	setWordLetters(buffer, word, letter);
	return messageType;
}

void setWordLetters(char buffer[], char word[], char letter) {
	int messageType = (int) buffer[0];
	if(messageType == GUESS_ANSWER_TYPE) {
		int occurrenceNumber = buffer[1];
		for(int i=0; i<occurrenceNumber; i++) {
			int letterPosition = (int) buffer[2+i];
			word[letterPosition] = letter;
		}
	} else if(messageType == END_GAME_TYPE) {
		for(int i=0; i<strlen(word); i++) {
			if(word[i] == '_') {
				word[i] = letter;
			}
		}
	} else {
		logExit("Error receiving message. (Invalid message type)");
	}
}