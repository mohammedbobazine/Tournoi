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

#include "../headers/protocol.h"
#include "../headers/validation.h"

/************************
*
* Serveur GAME
* BOUBAZINE MOHAMMED
*
*************************/
#define TIME_MAX 6
#define TNOM 30





int main(int argc, char** argv) {
    int err;
    int portServeur;
    int sockConx;
    int sizeAddr;
    
    int Trans1;
    int Trans2;



    fd_set readSet;

    int nfsd = 6;



    /*
    * verification des arguments
    */

    if (argc != 2) {
        printf("usage :  %s portServeur no port\n", argv[0]);
        return -1;

    }
    portServeur = atoi(argv[1]);

    printf("start serveur : %s num du port %s ...", argv[0], argv[1]);

    sizeAddr = sizeof(struct sockaddr_in);
   
   /*
   Creation de la socket de connexion
   */
    sockConx = socketServeur(portServeur); 
    if (sockConx < 0) {
        return -1;
    }

    /*
    socket de transmission pour le premier joueur a ce connecter 
    */
    Trans1 = accept(sockConx, NULL, NULL);
    if (Trans1 < 0) {
        printf("(serveur) erreur sur le accept du premier joueur \n ");
        return -2;
    }
    /*
    socket de transmission pour le deuxieme joueur 
    */
    Trans2 = accept(sockConx, NULL, NULL);
    if (Trans2 < 0) {
        printf("(serveur) erreur sur le accept 2eme joueur \n");
    }

    /* remetre a zero */
    FD_ZERO(&readSet);
    FD_SET(Trans1, &readSet);
    FD_SET(Trans2, &readSet);


    /*****************************************************
    * 
    * PARTIE RECEPTION DES REQUETE DE PARTIE & AFFECTATION DES PION ET ADVERSAIRE
    * 
    ******************************************************/

    err = select(nfsd, &readSet, NULL, NULL, NULL);
    
    if (err < 0) {
        printf("(serveur) erreur sur le select \n");
        return -3;
    }
    /*
    pour l'ordre de reception des requete
    */
    bool NORecu1 = true;
    bool NORecu2 = false;

    /*type de requete */  
    TIdRequest type_requete;
    /* requete de la partie  */
    TPartieReq requetePartie;
    /* reponse */
    TPartieRep reponsePartie1;
    TPartieRep reponsePartie2;

    /*nom des joueur*/
    char joueur1[TNOM];
    char joueur2[TNOM];


    /*sortire de la boucle lors de la reception des deux requete*/
    bool sortireBoucle = false;
    while (!sortireBoucle) {

        /* remetre a zero */
        FD_ZERO(&readSet);
        FD_SET(Trans1, &readSet);
        FD_SET(Trans2, &readSet);

        if (FD_ISSET(Trans1, &readSet) !=0 && NORecu1) {

            /*
            reception du code de la requete avec MSG_PEEK
            */
            err = recv(Trans1, &type_requete, sizeof(TIdRequest), MSG_PEEK);
            if (err < 0) {
                printf("(serveur) erreur sur la reception de l'ID de la requete sur Trans1 \n");
            }
            switch (type_requete)
            {
            case PARTIE:
                /* reception de la requete partie*/
                err = recv(Trans1, &requetePartie, sizeof(TPartieReq), 0);

                reponsePartie1.err = ERR_OK;
                reponsePartie1.coul = BLANC;
                /*recuperation du nom du prermier joueur*/
                memcpy(joueur1, requetePartie.nomJoueur, TNOM);
                break;

            default:
                /*envoi du code d'erreur*/
                reponsePartie1.err = ERR_TYP;

                break;
            }
            /* enversemant des acceé pour permetre la reception de la deuxieme requete */

            NORecu1 = false;
            NORecu2 = true;

        }

        if (FD_ISSET(Trans2, &readSet) !=0 && NORecu2) {

                 /*
            reception du code de la requete avec MSG_PEEK
               */
            err = recv(Trans2, &type_requete, sizeof(TIdRequest), MSG_PEEK);
            if (err < 0) {
                printf("(serveur) erreur sur la reception de l'ID de la requete sur Trans2 \n");
            }
            switch (type_requete)
            {
            case PARTIE:
                /* reception de la requete partie*/
                err = recv(Trans2, &requetePartie, sizeof(TPartieReq), 0);

                reponsePartie2.err = ERR_OK;
                reponsePartie2.coul = NOIR;
                /*recuperation du nom du deuxieme joueur*/
                memcpy(joueur1, requetePartie.nomJoueur, TNOM);
                break;

            default:
                /*envoi du code d'erreur*/
                reponsePartie2.err = ERR_TYP;

                break;
            }
            NORecu2 = false;
            sortireBoucle = true; // pour sortire de la boucle;
        }


    }

    /* 
    envoi des advarsaire au joueur
    */
    if (sortireBoucle) {

        /*affectation des advarsaire de chaque joueur */

        memcpy(reponsePartie1.nomAdvers, joueur2, TNOM);
        memcpy(reponsePartie2.nomAdvers, joueur1, TNOM);

        err = send(Trans1, &reponsePartie1, sizeof(TPartieRep), 0);
        if (err <= 0) {
            printf("(serveur) erreur sur l'envoi des advarsaire sur Trans1 \n");
            shutdown(Trans1, SHUT_RDWR);
            close(Trans1);
            return -5;
        }
        err = send(Trans2, &reponsePartie2, sizeof(TPartieRep), 0);
        if (err <= 0) {
            printf("(serveur) erreur sur l'envoi des advarsaire sur Trans2 \n");
            shutdown(Trans2, SHUT_RDWR);
            close(Trans2);
            return -5;
        }
    }
    /******************************************
    * 
    * DEMARAGE DE LA PREMIERE PARTIE
    * 
    *******************************************/

    int NbrePartie = 1;//numero de la partie
    initialiserPartie();
    printf("----------- Debut de la premier partie ---------------\n6|\n|\n|\n|\n|-------------------------------------------------------- \n");















}


