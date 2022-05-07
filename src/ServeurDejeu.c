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
#include "../fonctions/headers/fonctionsTCP.h"

#include "../headers/protocol.h"

/************************
*
* Serveur de jeu
* BOUBAZINE MOHAMMED
*
*************************/
#define MAX_JOUEUR 1 // nombre de joueures dans la partie
#define TIME_MAX 6
#define TNOM 30

int traitRequete(int socketJ1 ,int socketJ2) {

    /*
    nom des deux joueur
    */
   char joueur1[TNOM];
   char joueur2[TNOM];
    int err;

    /* request  PARTIE*/

     TPartieReq reqPartieJ1;
     TPartieReq reqPartieJ2;

    /* reponse PARTIE */
     TPartieRep reponseJ1;
     TPartieRep reponseJ2;


    err = recv(socketJ1, &reqPartieJ1, sizeof(  TPartieReq), 0);
    if (err < 0) {
        
        perror("(serveur) erreur sur la recption de requete de la partie");
        return -6;
    }
    err = recv(socketJ2, &reqPartieJ2, sizeof(  TPartieReq), 0);
    if (err < 0) {

        perror("(serveur) erreur sur la recption de requete de la partie");
        return -6;
    }
    /*
    * 
    * 
    */

    if (reqPartieJ1.idRequest == PARTIE && reqPartieJ2.idRequest == PARTIE) {
       
       
        joueur1 = reqPartieJ1.nomJoueur;
        joueur2 = reqPartieJ2.nomJoueur;
       
      
        
   

        reponseJ1.err = ERR_OK;
        reponseJ1.coul = BLANC;
        reponseJ1.nomAdvers = reqPartieJ2.nomJoueur;

        reponseJ2.err = ERR_OK;
        reponseJ2.coul = NOIR;
        reponseJ2.nomAdvers = reqPartieJ1.nomJoueur;

        /* envoi du nom de l'adversaire de chaque joueur */

        err = send(socketJ1, &reponseJ1, sizeof( TPartieRep), 0);
        if (err < 0) {
            perror("(serveur) erruer sur l'envoi des adversaire");
            return -5;
        }

        err = send(socketJ2, &reponseJ2, sizeof( TPartieRep), 0);
        if (err < 0) {
            perror("(serveur) erruer sur l'envoi des adversaire");
            return -5;
        }


    }
    else {
        /*
        envoi de l'erreur
        */
        reponseJ1.err = ERR_PARTIE;
        reponseJ2.err = ERR_PARTIE;

        err = send(socketJ2, &reponseJ2, sizeof( TPartieRep), 0);
        err = send(socketJ1, &reponseJ1, sizeof( TPartieRep), 0);
        return -1;
    }








}



int main(int argc, char** argv) {
    int err;
    int portServeur;
    int sockConx;
    int sizeAddr;
    struct sockaddr_in joueur1;
    int sockets[MAX_JOUEUR]; 
    //les sockets de trans.. avec le nombre de joueures
    /* noms des deux joueur de la partie */





    /*
    * verification des arguments
    */

    if (argc != 2) {
        printf("usage : %s portServeur\n", argv[0]);
        return -1;
    }

    portServeur = atoi(argv[1]);

    sockConx = socketServeur(portServeur); // socket de connexion du serveur
    if (sockConx < 0) {
        return -1;
    }

    sizeAddr = sizeof(struct sockaddr_in);

   
  

    /*
    * sockets de trans avec les joueur
    */


    int nbrecu = 0;
    while (1) {
        sockets[nbrecu] = accept(sockConx, (struct sockaddr*)&joueur1, (socklen_t*)&sizeAddr);
        if (sockets[nbrecu] > 0 && nbrecu < 2) {
            nbrecu++;
        }
        else if(nbrecu >= 2) {
            break;
        }

        if (sockets[nbrecu] < 0) {
            perror("(serveur) erreur sur accept du premier joueur");
            return -5;
        }
    }

    if (0 < traitRequete(sockets[0], sockets[1])) {
        shutdown(sockets[0],SHUT_RDWR);
        close(sockets[0] );
        shutdown(sockets[1], SHUT_RDWR);
        close(sockets[1]);

   }

   
    




    
}


