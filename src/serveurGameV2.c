#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <stdbool.h>
#include "../fonctions/headers/fonctionsTCP.h"
#include "time.h"

#include "../headers/protocol.h"
#include "../fonctions/headers/validation.h"

/************************
*
* Serveur GAME
* BOUBAZINE MOHAMMED
*
*************************/
#define TIME_MAX 6
#define TNOM 30

void verifArgum(bool validation, bool timeout, int port) {

    if (timeout && validation) {
        printf("START sur le port %d avec timeout et avec validation \n", port);
    }
    else  if (timeout && !validation) {
        printf("START sur le port %d avec timeout et sans validation \n", port);
    }
    else if (!timeout && validation) {
        printf("START sur le port %d sans timeout et avec validation \n", port);
    }
    else if (!timeout && !validation) {
        printf("START sur le port %d sans timeout et sans validation \n", port);
    }

}



int main(int argc, char** argv) {
    int err;
    int portServeur;
    int sockConx;
    int sizeAddr;

    int Trans1;
    int Trans2;

    fd_set readSet;
    int port;
    int nfsd = 6;


    // verification des argumemnts
    if (argc != 2) {
        printf("usage :  %s portServeur no port\n", argv[0]);
        return -1;

    }
    portServeur = atoi(argv[1]);

    sizeAddr = sizeof(struct sockaddr_in);

    //creation de la socket de connexion
    sockConx = socketServeur(portServeur);
    if (sockConx < 0) {
        return -1;
    }
    //printf("START serveur : %s sur le port N: %s ...\n", argv[0], argv[1]);
    

    char* time = "--noTimeout";//pas de timeout
    char* valid = "--novalid";//pad de validation
    //verification des argumment 
    bool timeout = true;
    bool validation = true;

    if (argc >= 2 || argc <= 4) {
        if (argc == 2) {
            port = atoi(argv[1]);
            timeout = false;
            validation = false;
        }


        else if (argc == 4 && (!(strcmp(argv[1], time)) || !(strcmp(argv[1], valid))) &&
            (!(strcmp(argv[2], time)) || !(strcmp(argv[2], valid))) && (strcmp(argv[1], argv[2]))) {
            port = atoi(argv[3]);
            if (!strcmp(argv[2], time) || !strcmp(argv[1], time)) {
                timeout = false;
            }
            if (!strcmp(argv[2], valid) || !strcmp(argv[1], valid)) {
                validation = false;
            }
        }
        else {
            printf("usage : ./serveur [--noValid|--noTimeout] no_port\n");
            return -2;
        }
    }
    
    else {
        printf("usage : ./serveur [--noValid|--noTimeout] no_port\n");
        return -2;
    }
    verifArgum(validation, timeout, port);

    
    


}



