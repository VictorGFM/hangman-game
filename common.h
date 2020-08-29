#pragma once

#include <stdlib.h>

#include <arpa/inet.h>

#define START_GAME_TYPE 1
#define GUESS_SEND_TYPE 2
#define GUESS_ANSWER_TYPE 3
#define END_GAME_TYPE 4

void logExit(const char *msg);

int addressParse(const char *addrstr, const char *portstr, struct sockaddr_storage *storage);

int serverAddressInitialize(const char *proto, const char *portstr, struct sockaddr_storage *storage);
