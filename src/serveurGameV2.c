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

        }
      
        else if (argc == 4) {
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

    memcpy(reponsePartie1.nomAdvers, joueur1, TNOM);
    memcpy(reponsePartie2.nomAdvers, joueur2, TNOM);


    err = send(Trans1, &reponsePartie1, sizeof(TPartieRep), 0);
    if (err <= 0) {
        perror("erreur : sur le send de reponsPartie1 sur Trans1\n");
        shutdown(Trans1, SHUT_RDWR);
        close(Trans1);
        return -5;

    }
    err = send(Trans2, &reponsePartie2, sizeof(TPartieRep), 0);
    if (err <= 0) {
        perror("erreur : sur le send de reponsPartie 2 sur Trans 2\n");
        shutdown(Trans2, SHUT_RDWR);
        close(Trans2);
        return -5;

    }

    /*******************************  PARTIE I :********************************/
    
    initialiserPartie();
    printf("Arbitre : partie I \n joueur 1 : %s PION [BLANC] \njoueur 2 : %s PION [NOIR] \nAttente du COUP du joueur 1...\n",joueur1,joueur2);

    while (NBpartie == 1) {
        if (timeout) {
            //config du timeout
            struct timeval tval1;
            tval1.tv_sec = TIME_MAX;
            tval1.tv_usec = 0;
            FD_ZERO(&readSet1);
            FD_SET(Trans1, &readSet1);

            TheTime = select(Trans1 + 1, &readSet1, NULL, NULL, &tval1);
        }
        if(!timeout || (TheTime >0 && timeout)){
            err = recv(Trans1, &idrequete, sizeof(TIdRequest), MSG_PEEK);
            if (err <= 0) {
                perror("erreur : sur le recv de idrequete  sur Trans 1\n");
                shutdown(Trans1, SHUT_RDWR);
                close(Trans1);
                return -5;
            }
            switch (idrequete)
            {
            case COUP:
                err = recv(Trans1, &requetecoup, sizeof(TCoupReq), 0);
                if (err <= 0) {
                    perror("erreur : sur le recv de requeteCoup  sur Trans 1\n");
                    shutdown(Trans1, SHUT_RDWR);
                    close(Trans1);
                    return -5;
                }
                printf("Arbitre : reception du [COUP] du joueur 1\n");
                bool coupValid = validationCoup(1, requetecoup, &propCoup);
                
                // A revoir validation !!
                if (validation && coupValid == false) {
                    reponsecoup.err = ERR_COUP;
                    reponsecoup.validCoup = TRICHE;
                    reponsecoup.propCoup = PERDU;
                    err = send(Trans1, &reponsecoup, sizeof(TCoupRep), 0);
                    if (err <= 0) {
                        perror("erreur : sur le send de [ERR_COUP]  sur Trans 1\n");
                        shutdown(Trans1, SHUT_RDWR);
                        close(Trans1);
                        return -5;
                    }
                    err = send(Trans2, &reponsecoup, sizeof(TCoupRep), 0);
                    if (err <= 0) {
                        perror("erreur : sur le send de [ERR_COUP]  sur Trans 2\n");
                        shutdown(Trans2, SHUT_RDWR);
                        close(Trans2);
                        return -5;
                    }
                    joueur1Nbperte++;
                    joueur2Nbgagne++;
                    NBpartie++;
                    break;
                }
                else {
                    printf("Arbitre: validation du [COUP] du joueur 1\n");
                    reponsecoup.err = ERR_OK;
                    reponsecoup.propCoup = propCoup;
                    reponsecoup.validCoup = VALID;
                }
                err = send(Trans1, &reponsecoup, sizeof(TCoupRep), 0);
                if (err <= 0) {
                    perror("erreur : sur le send de propCoup  sur Trans 1\n");
                    shutdown(Trans1, SHUT_RDWR);
                    close(Trans1);
                    return -5;
                }
                err = send(Trans2, &reponsecoup, sizeof(TCoupRep), 0);
                if (err <= 0) {
                    perror("erreur : sur le send de propCoup  sur Trans 2\n");
                    shutdown(Trans2, SHUT_RDWR);
                    close(Trans2);
                    return -5;
                }

                if (coupValid == true && propCoup != COUP) {
                    switch (propCoup)
                    {
                    case GAGNE:
                        joueur1Nbgagne++;
                        joueur2Nbperte++;
                        NBpartie++;
                        break;
                    case NULLE:
                        joueur1Nulle++;
                        joueur2Nulle++;
                        NBpartie++;
                        break;
                    case PERDU:
                        joueur2Nbgagne++;
                        joueur1Nbperte++;
                        NBpartie++;
                        break;
                    default:
                        break;
                    }
                }
                if ((coupValid == true ) || ( !validation)) {
                    err = send(Trans2, &requetecoup, sizeof(TCoupReq), 0);
                    if (err <= 0) {
                        perror("erreur : sur le send de requete sur Trans 2\n");
                        shutdown(Trans2, SHUT_RDWR);
                        close(Trans2);
                        return -5;
                    }
                    printf("Arbitre : envoi a l'adversaire..\n");
                }
                break;

            default:

                reponsecoup.err = ERR_TYP;
                err = send(Trans1, &reponsecoup, sizeof(TCoupRep), 0);
                if (err <= 0) {
                    perror("erreur : sur le send de [ERR_TYP]  sur Trans 1\n");
                    shutdown(Trans1, SHUT_RDWR);
                    close(Trans1);
                    return -5;

                }
                joueur1Nbperte++;
                joueur2Nbgagne++;
                NBpartie++;
                break;
            }
            if (NBpartie != 1) {
                break;
            }

            /*++++++ COUP JOUEUR 2++++++*/

            if (timeout) {
                //config du timeout
                struct timeval tval2;
                tval2.tv_sec = TIME_MAX;
                tval2.tv_usec = 0;
                FD_ZERO(&readSet1);
                FD_SET(Trans2, &readSet1);

                TheTime = select(Trans2 + 1, &readSet1, NULL, NULL, &tval2);
            }
            if ((TheTime > 0 && timeout) || !timeout) {
                
                err = recv(Trans2, &idrequete, sizeof(TIdRequest), MSG_PEEK);
                if (err <= 0) {
                    printf("erreur: sur le recv idrequete sur Trans2\n");
                    shutdown(Trans2, SHUT_RDWR);
                    close(Trans2);
                    return -5;
                }
                switch (idrequete)
                {
                case COUP:
                    err = recv(Trans2, &requetecoup, sizeof(TCoupReq), 0);
                    
                    if (err <= 0) {
                        printf("erreur: sur le recv requetecoup sur Trans2\n");
                        shutdown(Trans2, SHUT_RDWR);
                        close(Trans2);
                        return -5;
                    }

                    printf("Arbitre : reception du [COUP] de la par du joueur 2\n");
                    bool coupvValid = validationCoup(2, requetecoup, &propCoup);

                    if (validation && coupvValid == false) {
                       
                        reponsecoup.err = ERR_COUP;
                        reponsecoup.propCoup = PERDU;
                        reponsecoup.validCoup = TRICHE;
                        
                        err = send(Trans2, &reponsecoup, sizeof(TCoupRep), 0);
                        if (err <= 0) {
                            printf("erreur: sur le send reponsecoup [ERR_COUP] sur Trans2\n");
                            shutdown(Trans2, SHUT_RDWR);
                            close(Trans2);
                            return -5;
                        }

                        err = send(Trans1, &reponsecoup, sizeof(TCoupRep), 0);
                        if (err <= 0) {
                            printf("erreur: sur le send reponsecoup [ERR_COUP] sur Trans1\n");
                            shutdown(Trans1, SHUT_RDWR);
                            close(Trans1);
                            return -5;
                        }
                        joueur2Nbperte++;
                        joueur1Nbgagne++;
                        NBpartie++;
                        break;

                    }
                    else {
                        printf("Arbitre : validation du [COUP] du joueur2\n");
                        reponsecoup.err = ERR_OK;
                        reponsecoup.propCoup = propCoup;
                        reponsecoup.validCoup = VALID;
                    }

                    err = send(Trans2, &reponsecoup, sizeof(TCoupRep), 0);
                    if (err <= 0) {
                        printf("erreur: sur le send reponsecoup [VALID] sur Trans2\n");
                        shutdown(Trans2, SHUT_RDWR);
                        close(Trans2);
                        return -5;
                    }

                    err = send(Trans1, &reponsecoup, sizeof(TCoupRep), 0);
                    if (err <= 0) {
                        printf("erreur: sur le send reponsecoup [VALID] sur Trans1\n");
                        shutdown(Trans1, SHUT_RDWR);
                        close(Trans1);
                        return -5;
                    }

                    if (coupvValid == true && propCoup != CONT) {
                        switch (propCoup)
                        {
                        case GAGNE:
                            joueur2Nbgagne++;
                            joueur1Nbperte++;
                            NBpartie++;
                            break;
                        case PERDU:
                            joueur1Nbgagne++;
                            joueur2Nbperte++;
                            NBpartie++;
                            break;
                        case NULLE:
                            joueur1Nulle++;
                            joueur2Nulle++;
                            NBpartie++;
                            break;

                        default:
                            break;
                        }
                    }
                    /*!!! a revoir la condition */
                    if ((coupvValid == true && propCoup == CONT) || (!validation && propCoup == CONT)) {
                        printf("Arbitre : envoi [COUP] a l'adrversaire joueur1 \n");
                        err = send(Trans1, &requetecoup, sizeof(TCoupReq), 0);
                        if (err <= 0) {
                            printf("erreur: sur le send requetecoup [COUP] sur Trans2\n");
                            shutdown(Trans1, SHUT_RDWR);
                            close(Trans1);
                            return -5;
                        }
                    }
                    break;


                default:
                    reponsecoup.err = ERR_TYP;
                    err = send(Trans2, &reponsecoup, sizeof(TCoupRep), 0);
                    if (err <= 0) {
                        printf("erreur: sur le send reponsecoup [ERR_TYP] sur Trans2\n");
                        shutdown(Trans2, SHUT_RDWR);
                        close(Trans2);
                        return -5;
                    }
                    err = send(Trans1, &reponsecoup, sizeof(TCoupRep), 0);
                    if (err <= 0) {
                        printf("erreur: sur le send reponsecoup [ERR_TYP] sur Trans1\n");
                        shutdown(Trans1, SHUT_RDWR);
                        close(Trans1);
                        return -5;
                    }
                    joueur1Nbgagne++;
                    joueur2Nbperte++;
                    NBpartie++;

                    break;
                }
            }

            if (TheTime == 0 && timeout) {
                printf("Arbitre: fin d'attent pour le joueur 2\nfin de partie 1\n");
                reponsecoup.err = ERR_COUP;
                reponsecoup.propCoup = PERDU;
                reponsecoup.validCoup = TIMEOUT;

                err = send(Trans1, &reponsecoup, sizeof(TCoupRep),0);
                if (err <= 0) {
                    printf("erreur: sur le send reponsecoup [TIMEOUT] sur Trans1\n");
                    shutdown(Trans1, SHUT_RDWR);
                    close(Trans1);
                    return -5;
                }
                err = send(Trans2, &reponsecoup, sizeof(TCoupRep), 0);
                if (err <= 0) {
                    printf("erreur: sur le send reponsecoup [TIMEOUT] sur Trans2\n");
                    shutdown(Trans2, SHUT_RDWR);
                    close(Trans2);
                    return -5;
                }
                joueur1Nbgagne++;
                joueur2Nbperte++;
                NBpartie++;
                printf("Arbitre :%s  a perdu la premier partie\n", joueur2);
                break;

            }
         }

         if (TheTime == 0 && timeout) {

             printf("Arbitre: fin d'attent pour le joueur 1\nfin de partie 1\n");
             reponsecoup.err = ERR_COUP;
             reponsecoup.propCoup = PERDU;
             reponsecoup.validCoup = TIMEOUT;

             err = send(Trans1, &reponsecoup, sizeof(TCoupRep), 0);
             if (err <= 0) {
                 printf("erreur: sur le send reponsecoup [TIMEOUT] sur Trans1\n");
                 shutdown(Trans1, SHUT_RDWR);
                 close(Trans1);
                 return -5;
             }
             err = send(Trans2, &reponsecoup, sizeof(TCoupRep), 0);
             if (err <= 0) {
                 printf("erreur: sur le send reponsecoup [TIMEOUT] sur Trans2\n");
                 shutdown(Trans2, SHUT_RDWR);
                 close(Trans2);
                 return -5;
             }
             joueur1Nbperte++;
             joueur2Nbgagne++;
             NBpartie++;
             printf("Arbitre :%s  a perdu la premier partie\n", joueur1);
             break;
         }
    }

    /*++++++++ FIN PARTIE I +++++++++ */

    /*++++++++  PARTIE II  ++++++++++*/

    initialiserPartie();
    printf("Arbitre : match retour\n");


    while (NBpartie == 2)
    {
        if (timeout) {
            FD_ZERO(&readSet1);
            FD_SET(Trans2, &readSet1);
            struct timeval tval3;
            tval3.tv_sec = TIME_MAX;
            tval3.tv_usec = 0;
            int TheTime = select(Trans2 + 1, &readSet1, NULL, NULL, &tval3);
        }
        if ((TheTime > 0 && timeout) || !timeout) {
            err = recv(Trans2, &idrequete, sizeof(TIdRequest), MSG_PEEK);
            if (err <= 0) {
                printf("erreur: sur le recv idrequest sur Trans2\n");
                shutdown(Trans2, SHUT_RDWR);
                close(Trans2);
                return -5;
            }

            switch (idrequete)
            {
            case COUP:
                err = recv(Trans2, &requetecoup, sizeof(TCoupReq), 0);
                if (err <= 0) {
                    printf("erreur: sur le recv requetecoup sur Trans2\n");
                    shutdown(Trans2, SHUT_RDWR);
                    close(Trans2);
                    return -5;
                }
                printf("Arbitre : reception du [COUP] du joueur 2\n");
                bool coupValid3 = validationCoup(1, requetecoup, &propCoup);

                if (coupValid3 == false && validation) {
                    reponsecoup.err = ERR_COUP;
                    reponsecoup.validCoup = TRICHE;
                    reponsecoup.propCoup = PERDU;

                    err = send(Trans2, &reponsecoup, sizeof(TCoupRep), 0);
                    if (err <= 0) {
                        printf("erreur: sur le send reponsecoup [ERR_COUP] sur Trans2\n");
                        shutdown(Trans2, SHUT_RDWR);
                        close(Trans2);
                        return -5;
                    }
                    err = send(Trans1, &reponsecoup, sizeof(TCoupRep), 0);
                    if (err <= 0) {
                        printf("erreur: sur le send reponsecoup [ERR_COUP] sur Trans1\n");
                        shutdown(Trans1, SHUT_RDWR);
                        close(Trans1);
                        return -5;
                    }

                    joueur1Nbgagne++;
                    joueur2Nbperte++;
                    NBpartie++;
                    break;

                }
                else {
                    printf("Arbitre : envoi  COUP [VALID] \n");
                    reponsecoup.err = ERR_COUP;
                    reponsecoup.propCoup = propCoup;
                    reponsecoup.validCoup = VALID;

                }

                err = send(Trans2, &reponsecoup, sizeof(TCoupRep), 0);
                if (err <= 0) {
                    printf("erreur: sur le send reponsecoup [VALID] sur Trans2\n");
                    shutdown(Trans2, SHUT_RDWR);
                    close(Trans2);
                    return -5;
                }

                err = send(Trans1, &reponsecoup, sizeof(TCoupRep), 0);
                if (err <= 0) {
                    printf("erreur: sur le send reponsecoup [VALID] sur Trans1\n");
                    shutdown(Trans1, SHUT_RDWR);
                    close(Trans1);
                    return -5;
                }

                if ((coupValid3 == true && propCoup == CONT) || (!validation && propCoup == CONT)) {
                    switch (propCoup)
                    {
                    case GAGNE :
                        joueur2Nbgagne++;
                        joueur1Nbperte++;
                        NBpartie++;
                        break;
                    case PERDU :
                        joueur1Nbgagne++;
                        joueur2Nbperte++;
                        NBpartie++;
                        break;
                    case NULLE:
                        joueur1Nulle++;
                        joueur2Nulle++;
                        NBpartie++;
                        break;
                    default:
                        break;
                    }
                }

                if ((coupValid3 == true && propCoup == CONT) || (!validation && propCoup == CONT))
                {
                    printf("Arbitre : envoi requtecoup au joueur1 \n");
                    err = send(Trans1, &requetecoup, sizeof(TCoupReq), 0);
                    if (err <= 0) {
                        printf("erreur: sur le send requetecoup sur Trans1\n");
                        shutdown(Trans1, SHUT_RDWR);
                        close(Trans1);
                        return -5;
                    }
                }
                break;

            default:
                reponsecoup.err = ERR_TYP;
                err = send(Trans2, &reponsecoup, sizeof(TCoupRep), 0);
                if (err <= 0) {
                    printf("erreur: sur le send reponsecoup [ERR_TYP] sur Trans2\n");
                    shutdown(Trans2, SHUT_RDWR);
                    close(Trans2);
                    return -5;
                }
                err = send(Trans1, &reponsecoup, sizeof(TCoupRep), 0);
                if (err <= 0) {
                    printf("erreur: sur le send reponsecoup [ERR_TYP] sur Trans1\n");
                    shutdown(Trans1, SHUT_RDWR);
                    close(Trans1);
                    return -5;
                }
                joueur2Nbperte++;
                joueur1Nbgagne++;
                NBpartie++;


                break;
            }
            /* !!!*/
            if (NBpartie >2) {
                break;
            }

            /*+++++++++ COUP JOUEUR 2 ++++++++++*/

            if (timeout) {
                printf("Arbitre : \n");
                FD_ZERO(&readSet1);
                FD_SET(Trans1, &readSet1);

                struct timeval tval4;
                tval4.tv_sec = TIME_MAX;
                tval4.tv_usec = 0;

                int Thetime = select(Trans1 + 1, &readSet1, NULL, NULL, &tval4);
                }

            if (!timeout || (TheTime > 0 && timeout)) {
                err = recv(Trans1, &idrequete, sizeof(TIdRequest), MSG_PEEK);
                if (err <= 0) {
                    printf("erruer : sur le recv de idrequete sur Trans1 \n");
                    shutdown(Trans1, SHUT_RDWR);
                    close(Trans1);
                    return -5;
                }
                switch (idrequete)
                {
                case COUP:
                    err = recv(Trans1, &requetecoup, sizeof(TCoupReq), 0);
                    if (err <= 0) {
                        printf("erruer : sur le recv de requetecoup sur Trans1 \n");
                        shutdown(Trans1, SHUT_RDWR);
                        close(Trans1);
                        return -5;
                    }
                    printf("Arbitre : reception requetecoup du joueur1  \n");

                    bool coupValid4 = validationCoup(2, requetecoup, &propCoup);
                    
                    if (coupValid4 == false && validation )
                    {
                        reponsecoup.err = ERR_COUP;
                        reponsecoup.propCoup = TRICHE;
                        reponsecoup.validCoup = PERDU;
                        
                        err = send(Trans1, &reponsecoup, sizeof(TCoupRep), 0);
                        if (err <= 0) {
                            printf("erruer : sur le send de reponsecoup [ERR_COUP] sur Trans1 \n");
                            shutdown(Trans1, SHUT_RDWR);
                            close(Trans1);
                            return -5;
                        }

                        err = send(Trans2, &reponsecoup, sizeof(TCoupRep), 0);
                        if (err <= 0) {
                            printf("erruer : sur le send de reponsecoup [ERR_COUP] sur Trans2 \n");
                            shutdown(Trans2, SHUT_RDWR);
                            close(Trans2);
                            return -5;
                        }
                        joueur1Nbperte++;
                        joueur2Nbgagne++;
                        NBpartie++;
                        break;

                    }
                    else {
                        printf("Arbitre : validation du [COUP] pour le joueur1\n");
                        reponsecoup.err = ERR_OK;
                        reponsecoup.propCoup = propCoup;
                        reponsecoup.validCoup = VALID;
                    }
                    err = send(Trans1, &reponsecoup, sizeof(TCoupRep), 0);
                   
                    if (err <= 0) {
                        printf("erruer : sur le send de reponsecoup [VALID] sur Trans1 \n");
                        shutdown(Trans1, SHUT_RDWR);
                        close(Trans1);
                        return -5;
                    }
                    err = send(Trans2, &reponsecoup, sizeof(TCoupRep), 0);

                    if (err <= 0) {
                        printf("erruer : sur le send de reponsecoup [VALID] sur Trans2 \n");
                        shutdown(Trans2, SHUT_RDWR);
                        close(Trans2);
                        return -5;
                    }

                    if (coupValid4 == true && propCoup != CONT) {
                        switch (propCoup)
                        {
                        case GAGNE:
                            joueur1Nbgagne++;
                            joueur2Nbperte++;
                            NBpartie++;
                            break;
                        case NULLE:
                            joueur1Nulle++;
                            joueur2Nulle++;
                            NBpartie++;
                            break;
                        case PERDU:
                            joueur1Nbperte++;
                            joueur2Nbgagne++;
                            NBpartie++;
                            break;
                        default:
                            break;
                        }
                    }

                    if ((coupValid4 == true && propCoup == CONT) || (!validation && propCoup == CONT)) {
                       
                        printf("Arbitre : envoi requtecoup au joueur2 \n");
                        err = send(Trans2, &requetecoup, sizeof(TCoupReq), 0);
                        if (err <= 0) {
                            printf("erruer : sur le send de requetecoup sur Trans2 \n");
                            shutdown(Trans2, SHUT_RDWR);
                            close(Trans2);
                            return -5;
                        }
                    }
                    break;

                default:

                    reponsecoup.err = ERR_TYP;
                    err = send(Trans1, &reponsecoup, sizeof(TCoupRep), 0);
                    if (err <= 0) {
                        perror("erreur : sur le send reponsecoup [ERR_TYP] sur Trans1 \n");
                        shutdown(Trans1, SHUT_RDWR);
                        close(Trans1);
                        return -5;
                    }
                    err = send(Trans2, &reponsecoup, sizeof(TCoupRep), 0);
                    if (err <= 0) {
                        perror("erreur : sur le send reponsecoup [ERR_TYP] sur Trans2 \n");
                        shutdown(Trans2, SHUT_RDWR);
                        close(Trans2);
                        return -5;
                    }
                    joueur2Nbgagne++;
                    joueur1Nbperte++;
                    NBpartie++;

                    break;
                }

            }

            if (TheTime == 0 && timeout) {
                reponsecoup.err = ERR_COUP;
                reponsecoup.validCoup = TIMEOUT;
                reponsecoup.propCoup = PERDU;
                printf("Arbitre : timeout depasser pour le joueur 1 \n");
                err = send(Trans1, &reponsecoup, sizeof(TCoupRep), 0);
                if (err <= 0) {
                    perror("erreur : sur le send reponsecoup [TIMEOUT] sur Trans1 \n");
                    shutdown(Trans1, SHUT_RDWR);
                    close(Trans1);
                    return -5;
                }
                err = send(Trans2, &reponsecoup, sizeof(TCoupRep), 0);
                if (err <= 0) {
                    perror("erreur : sur le send reponsecoup [TIMEOUT] sur Trans2 \n");
                    shutdown(Trans2, SHUT_RDWR);
                    close(Trans2);
                    return -5;
                }
                joueur1Nbperte++;
                joueur2Nbgagne++;
                NBpartie++;
                break;

            }

        }
        if (TheTime == 0 && timeout) {
            reponsecoup.err = ERR_COUP;
            reponsecoup.validCoup = TIMEOUT;
            reponsecoup.propCoup = PERDU;
            printf("Arbitre : timeout depasser pour le joueur 2 \n");
            err = send(Trans1, &reponsecoup, sizeof(TCoupRep), 0);
            if (err <= 0) {
                perror("erreur : sur le send reponsecoup [TIMEOUT] sur Trans1 \n");
                shutdown(Trans1, SHUT_RDWR);
                close(Trans1);
                return -5;
            }
            err = send(Trans2, &reponsecoup, sizeof(TCoupRep), 0);
            if (err <= 0) {
                perror("erreur : sur le send reponsecoup [TIMEOUT] sur Trans2 \n");
                shutdown(Trans2, SHUT_RDWR);
                close(Trans2);
                return -5;
            }
            joueur1Nbgagne++;
            joueur2Nbperte++;
            NBpartie++;
            break;
        }
       }
       printf("                             FIN DU JEU                           \n");
       printf("  NOM JOUEUR [%s] || PARTIES GAGNES [%d]  || PARTIES PERDUS [%d]  || PARTIES NULS [%d] \n",joueur1,joueur1Nbgagne,joueur1Nbperte,joueur1Nulle);
       printf("  NOM JOUEUR [%s] || PARTIES GAGNES [%d]  || PARTIES PERDUS [%d]  || PARTIES NULS [%d] \n",joueur2,joueur2Nbgagne,joueur2Nbperte,joueur2Nulle);
       shutdown(Trans1, SHUT_RDWR);
       close(Trans1);
       shutdown(Trans2, SHUT_RDWR);
       close(Trans2);
       close(sockConx);
       return 0;


}





