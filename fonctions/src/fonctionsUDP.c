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

/*
* regroupe: socket + bind
* retourne: socket locale (émetteur + récepteur)
*/
int socketUDP(short noPort) {
    int sock,               /* descripteur de la socket locale */
      sizeAddr, 	  /* taille de l'adresse d'une socket */
      err;                /* code d'erreur */
    
    struct sockaddr_in adrLocal;	  /* adresse de la socket locale */
    
    /* creation de la socket, protocole UDP */
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("(emetteur) erreur de socket");
        return -2;
    }

    /* 
    * initialisation de l'adresse de la socket 
    */
    adrLocal.sin_family = AF_INET;
    adrLocal.sin_port = htons(noPort);
    adrLocal.sin_addr.s_addr = INADDR_ANY;
        // INADDR_ANY : 0.0.0.0 (IPv4) donc htonl inutile ici, car pas d'effet
    bzero(adrLocal.sin_zero, 8);

    sizeAddr = sizeof(struct sockaddr_in);
 
    /* 
    * attribution de l'adresse a la socket
    */
    err = bind(sock, (struct sockaddr *)&adrLocal, sizeAddr);
    if (err < 0) {
        perror("erreur sur le bind");
        return -3;
    }

    return sock;
}

/*
permet d'initialiser une adresse de socket (sur une machine donnée
et associée à un port donné). Le passage de cette adresse est fait par
pointeur (struct sockaddr_in*) puisque le contenu de la structure 
doit-être modifié.
*/
int adresseUDP(char* nomMachine, ushort port, struct sockaddr_in* addr) {
    int err;

    struct addrinfo hints;
    struct addrinfo *infos;
    
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = 0;
    hints.ai_protocol = 0;

    char portString[5];
    sprintf(portString, "%d", port);

    err = getaddrinfo(nomMachine, portString, &hints, &infos);
    if(err != 0) {
        perror("erreur sur getaddrinfo");
        return -3;
    }

    addr->sin_family = AF_INET;
    addr->sin_addr = ((struct sockaddr_in*)infos->ai_addr)->sin_addr;
    /*err = inet_aton(infos->ai_addr, &addr->sin_addr);
    if (err == 0) { 
        perror("erreur obtention IP");
        return -4;
    }*/
    addr->sin_port = htons(port);
    bzero(addr->sin_zero, 8);
}

/*
équivalent de adresseUDP(char* nomMachine, ushort port, struct sockaddr_in* addr)
*/
// !!!!!!! il faudrait utiliser un malloc ou qqch comme ça
/*struct sockaddr_in* initAddr(char* nomMachine, ushort nPort) {
    int err;
    struct sockaddr_in addr;

    addr.sin_family = AF_INET;
    err = inet_aton(nomMachine, &addr.sin_addr);
    if (err == 0) { 
        perror("erreur obtention IP");
        return -4;
    }
    addr.sin_port = htons(nPort);
    bzero(addr.sin_zero, 8);

    return &addr;
}*/