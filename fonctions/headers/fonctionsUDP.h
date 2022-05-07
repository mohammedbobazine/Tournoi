/* inclusions standards */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

/* inclusions socket */
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

#include <unistd.h>

int socketUDP(short noPort);
int adresseUDP(char* nomMachine, ushort port, struct sockaddr_in* addr);