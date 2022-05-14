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
* Serveur GAME VERSION 2
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

int selectioPartie(TPartieRep *reponsePartie1, TPartieRep *reponsePartie2, int Trans1, int Trans2, char(*joueur1)[TNOM], char(*joueur2)[TNOM]) {
    // !! REMARQUE : a revoir
    int nfsd = 6;
    fd_set readSet;
    bool Partie = false;
    bool enAttent1 = true;
    bool enAttent2 = false;
    int err;
    TIdRequest idrequete;
  /*  TPartieRep reponsePartie1;
    TPartieRep reponsePartie2;*/
    TPartieReq requetePartie;

    while (!Partie)
    {
        FD_ZERO(&readSet); // mise a zero
        FD_SET(Trans1, &readSet);
        FD_SET(Trans2, &readSet);

        if (FD_ISSET(Trans1, &readSet) != 0 && enAttent1) {
            err = recv(Trans1, &idrequete, sizeof(TIdRequest), MSG_PEEK);
            if (err <= 0) {
                perror("erreur : sur le recv idrequete sur Trans1 \n");
                return -5;
            }
            switch (idrequete)
            {
            case PARTIE:
                err = recv(Trans1, &requetePartie, sizeof(TPartieRep), 0);
                if (err <= 0) {
                    perror("erreur : sur le recv requetePartie sur Trans1 \n");
                    return -5;
                }
                reponsePartie1->err = ERR_OK;
                reponsePartie1->coul = BLANC;
                memcpy(joueur1, requetePartie.nomJoueur, TNOM);
                break;
            default:
                reponsePartie1->err = ERR_TYP;
                break;
            }
            enAttent1 = false;
            enAttent2 = true;

        }

        if (FD_ISSET(Trans2, &readSet) != 0 && enAttent2) {
            err = recv(Trans2, &idrequete, sizeof(TIdRequest), MSG_PEEK);
            if (err <= 0) {
                perror("erreur : sur le recv idrequete sur Trans2 \n");
                return -5;
            }
            switch (idrequete)
            {
            case PARTIE:
                err = recv(Trans2, &requetePartie, sizeof(TPartieRep), 0);
                if (err <= 0) {
                    perror("erreur : sur le recv requetePartie sur Trans1 \n");
                    return -5;
                }
                reponsePartie2->err = ERR_OK;
                reponsePartie2->coul = NOIR;
                memcpy(joueur2, requetePartie.nomJoueur, TNOM);
                break;
            default:
                reponsePartie2->err = ERR_TYP;
                break;
            }
            enAttent2 = true;
            // fin de l'affectation des PION
            Partie = true;
        }

    }
    return 0;
}

int main(int argc, char** argv) {

    int port;
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
    //affichage des argument de demarage
    verifArgum(validation, timeout, port);

    //Quelque Declaration
    int err;
    int sockConx;
    int sizeAddr;
    int Trans1;
    int Trans2;
    int nfsd = 6;
    fd_set readSet;
    fd_set readSet1;

    TIdRequest idrequete;
    
    TCoupRep reponsecoup;
    TCoupReq requetecoup;

    TPartieRep reponsePartie1;
    TPartieRep reponsePartie2;
    TPartieReq requetePartie;
 
    TPropCoup propCoup;

    char joueur1[TNOM];
    char joueur2[TNOM];

    int NBpartie = 1;

    int TheTime; 
    int joueur1Nbperte = 0;
    int joueur1Nbgagne = 0;
    int joueur2Nbperte = 0;
    int joueur2Nbgagne = 0;

    int joueur1Nulle = 0;
    int joueur2Nulle = 0;

    struct sockaddr_in addClient;
    sizeAddr = sizeof(struct sockaddr_in);

    //creation de la socket de connexion
    sockConx = socketServeur(port);
    if (sockConx < 0) {
        printf("erreur sur sockCon\n");
        return -1;
    }

    Trans1 = accept(sockConx, NULL, NULL);
    if (Trans1 < 0) {
        printf("erreur : sur accept de la trans 1 \n");
        return -2;
    }

    Trans2 = accept(sockConx, NULL, NULL);
    if (Trans2 < 0) {
        printf("erreur : sur accept de la trans 2 \n");
        return -2;
    }

    FD_ZERO(&readSet); // mise a zero
    FD_SET(Trans1, &readSet);
    FD_SET(Trans2, &readSet);

    err = select(nfsd, &readSet, NULL, NULL, NULL);
    if (err < 0) {
        printf("erreur : select \n");
        return -3;
    }
    // quelque problemes dans les parametres reverifier
    selectioPartie(&reponsePartie1, &reponsePartie2, Trans1, Trans2, &joueur1, &joueur2);





}





