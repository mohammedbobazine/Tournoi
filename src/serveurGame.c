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

    sizeAddr = sizeof(struct sockaddr_in);
   
   /*
   Creation de la socket de connexion
   */
    sockConx = socketServeur(portServeur); 
    if (sockConx < 0) {
        return -1;
    }
    printf("start serveur : %s num du port %s ...\n", argv[0], argv[1]);
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
    printf("Debut de la premier partie \n");
    
    
    int theTime;
    
    TCoupReq couprequete;
    TCoupRep coupreponse;
    fd_set readSet1;
    TPropCoup propCoup;

    /* intialisation du score de chaque joueur */
    int joueur1Nbperte = 0;
    int joueur1Nbgagne = 0;
    int joueur2Nbperte = 0;
    int joueur2Nbgagne = 0;
    int joueur1NULLE = 0;
    int joueur2NULLE = 0;


    
    while (NbrePartie == 1) {

        /*time 6 seconde*/
        struct timeval tval1;
        tval1.tv_sec = TIME_MAX; // Number of whole seconds of elapsed time
        tval1.tv_usec = 0;// Number of microseconds of rest of elapsed time minus tv_sec. Always less
        
        FD_ZERO(&readSet1);
        FD_SET(Trans1, &readSet1);
        /*Pour le timeout*/
        theTime = select(Trans1 + 1, &readSet1, NULL, NULL, &tval1);

        if (theTime > 0) {
            /* pour connaitre le type de requete*/
            err = recv(Trans1, &type_requete, sizeof(TIdRequest), MSG_PEEK);

            switch (type_requete)
            {
                /* le cas ou la requete est de type COUP*/
            case COUP:
                
                err = recv(Trans1, &couprequete, sizeof(TCoupReq), 0);

                if (err<0)
                {
                    perror("(serveur) erreur sur la reception du coup sur Trans1\n");
                    shutdown(Trans1, SHUT_RDWR);
                    close(Trans1);
                    return -5;

                }
                printf("(serveur) reception  -COUP- du joueur 1 [BLANC] ..\n");

                bool validcoup = validationCoup(1, couprequete, &propCoup);

                if (validcoup == false ) {
                    coupreponse.err = ERR_COUP;
                    coupreponse.validCoup = TRICHE;
                    coupreponse.propCoup = PERDU;

                    err = send(Trans1, &coupreponse, sizeof(TCoupRep), 0);
                    if (err <= 0) {
                        perror("(serveur) erreur sur le send Trans1 \n");
                        shutdown(Trans1, SHUT_RDWR);
                        close(Trans1);
                        return -5;
                    }
                    /*
                    *envi pour le 2eme joueur
                    */
                    err = send(Trans2, &coupreponse, sizeof(TCoupRep), 0);
                    if (err <= 0) {
                        perror("(serveur) erreur sur le send Trans2 \n");
                        shutdown(Trans2, SHUT_RDWR);
                        close(Trans2);
                        return -5;
                    }
                    /*mise a jour du score et passage a la deuxieme partie*/
                    joueur1Nbperte++;
                    joueur2Nbgagne++;
                    NbrePartie++;
                    /*sortir dans le cas la perte*/
                    break;
                }
                /* si le coup est valid alors..*/
                else {
                    printf("********************\n(serveur) COUP [valid]\n ********************\n");
                    coupreponse.err = ERR_OK;
                    coupreponse.validCoup = VALID;
                    coupreponse.propCoup = propCoup;


                }
                err = send(Trans1, &coupreponse, sizeof(TCoupRep), 0);
                if (err <= 0) {
                    perror("(serveur) erreur sur le send Trans1 \n");
                    shutdown(Trans1, SHUT_RDWR);
                    close(Trans1);
                    return -5;
                }
                /*
                *envi pour le 2eme joueur
                */
                err = send(Trans2, &coupreponse, sizeof(TCoupRep), 0);
                if (err <= 0) {
                    perror("(serveur) erreur sur le send Trans2 \n");
                    shutdown(Trans2, SHUT_RDWR);
                    close(Trans2);
                    return -5;
                }
                /*le cas GAGNANT OU PERDANT OU NUL*/
                if (validcoup && propCoup != CONT) {
                    switch (propCoup)
                    {
                    case GAGNE :
                        joueur2Nbperte++;
                        joueur1Nbgagne++;
                        NbrePartie++;
                        break;
                    case PERDU :
                        joueur1Nbperte++;
                        joueur2Nbgagne++;
                        NbrePartie++;
                        break;
                    case NULLE :
                        joueur1NULLE++;
                        joueur2NULLE++;
                        NbrePartie++;
                        break;
                    default:
                        break;
                    }

                }

                if (validcoup && propCoup == CONT) {
                    /* envoi de la requete au deuxieme joueur*/
                    err = send(Trans2, &couprequete, sizeof(TCoupReq), 0);
                    if (err <= 0) {
                        perror("(serveur) erreur sur le send Trans2 \n");
                        shutdown(Trans2, SHUT_RDWR);
                        close(Trans2);
                        return -5;
                    }
                }
                break;


                /*
                Code different de COUP
                Cas d'erreur
                */
            default:
                coupreponse.err = ERR_TYP;
                err = send(Trans1, &coupreponse, sizeof(TCoupRep), 0);
                if (err <= 0) {
                    perror("(serveur) erreur sur le send Trans1 \n");
                    shutdown(Trans1, SHUT_RDWR);
                    close(Trans1);
                    return -5;
                }
                /*passage a la deuxieme partie*/
                NbrePartie++;

                break;
            }
            /*ne pas receptionner le coup de l'adversaire dans le cas de fin de partie*/
            if (NbrePartie > 1) {
                break;
            }

            /*
            *
            * COUP du deuxieme joueur
            * 
            */
            /*time 6 seconde*/
            struct timeval tval2;
            tval1.tv_sec = TIME_MAX; // Number of whole seconds of elapsed time
            tval1.tv_usec = 0;// Number of microseconds of rest of elapsed time minus tv_sec. Always less

            FD_ZERO(&readSet1);
            FD_SET(Trans2, &readSet1);
            /*Pour le timeout*/
            int theTime = select(Trans2 + 1, &readSet1, NULL, NULL, &tval2);
            if (theTime > 0) {
                err = recv(Trans2, &type_requete, sizeof(TIdRequest), MSG_PEEK);
                if (err <= 0) {
                    perror("(serveur) erreur sur la reception du code de requete du deuxiemem joueur\n");
                    shutdown(Trans2, SHUT_RDWR);
                    close(Trans2);
                    return -5;
                }
                switch (type_requete)
                {
                case COUP:
                    err = recv(Trans2, &couprequete, sizeof(TCoupReq), 0);
                    if (err <= 0) {
                        perror("(serveur) erreur sur la reception de la requete de COUP du deuxieme joueur\n");
                        shutdown(Trans2, SHUT_RDWR);
                        close(Trans2);
                        return -5;
                    }

                    printf("(serveur) reception de la requete [COUP] du deuxieme joueur \n");

                    bool validation2 = validationCoup(2, couprequete, &propCoup);
                    if (validation2 == false ) {
                        coupreponse.err = ERR_COUP;
                        coupreponse.validCoup = TRICHE;
                        coupreponse.propCoup = PERDU;
                        err = send(Trans2, &coupreponse, sizeof(TCoupRep), 0);
                        if (err <= 0) {
                            perror("(serveur) erreur sur le send de la reponse de ERR_COUP au deuxieme joueur\n");
                            shutdown(Trans2, SHUT_RDWR);
                            close(Trans2);
                            return -5;
                        }
                        /*envoi de la reponse a l'adversair aussi*/
                        err = send(Trans1, &coupreponse, sizeof(TCoupRep), 0);
                        if (err <= 0) {
                            perror("(serveur) erreur sur le send de la reponse de ERR_COUP au premier joueur\n");
                            shutdown(Trans1, SHUT_RDWR);
                            close(Trans1);
                            return -5;
                        }
                        /*
                        fin de partie et passage a la deuxieme partie de jeu
                        */
                        joueur2Nbperte++;
                        joueur1Nbgagne++;
                        NbrePartie++;
                        break;

                        
                    }
                    /*si le coup est valid*/
                    else {
                        /* perparation de la reponse dans le cas ou le COUP est valid*/
                        coupreponse.err = ERR_OK;
                        coupreponse.validCoup = VALID;
                        coupreponse.propCoup = propCoup;

                    }
                    err = send(Trans2, &coupreponse, sizeof(TCoupRep), 0);
                    if (err <= 0) {
                        perror("(serveur) erreur sur le send de la reponse de COUP au deuxieme joueur\n");
                        shutdown(Trans2, SHUT_RDWR);
                        close(Trans2);
                        return -5;
                    }
                    /*envoi de la reponse a l'adversair*/
                    err = send(Trans1, &coupreponse, sizeof(TCoupRep), 0);
                    if (err <= 0) {
                        perror("(serveur) erreur sur le send de la reponse de COUP au premier joueur\n");
                        shutdown(Trans1, SHUT_RDWR);
                        close(Trans1);
                        return -5;
                    }




                    if (validation2 == true && propCoup != CONT) {
                switch (propCoup){
                case  PERDU:
                    joueur1Nbgagne++;
                    joueur2Nbperte++;
                    NbrePartie++;
                    break;
                case GAGNE:
                    joueur2Nbgagne++;
                    joueur1Nbperte++;
                    NbrePartie++;
                    break;
                case NULLE:
                    joueur1NULLE++;
                    joueur2NULLE++;
                    NbrePartie++;
                    break;
                    
                default:
                    break;
                            
                        }
                    }


                if (validation2 == true && propCoup == CONT) {
                    printf("(serveur) envoi du COUP a l'adversaire\n");
                    err = send(Trans1, &couprequete, sizeof(TCoupReq), 0);
                    if (err <= 0) {
                        perror("(serveur) erreur sur le send de la requette coup a Trans1\n");
                        shutdown(Trans1, SHUT_RDWR);
                        close(Trans1);
                        return -5;
                    }
                }
                break;


                default:

                    coupreponse.err = ERR_TYP;
                    err = send(Trans2, &coupreponse, sizeof(TCoupRep), 0);
                    if (err <= 0) {
                        perror("(serveur) erreur sur le send du code d'erreur a Trans2\n");
                        shutdown(Trans2, SHUT_RDWR);
                        close(Trans2);
                        return -5;
                    }
                    NbrePartie++;

                    break;
                }
                    
            }
            if (theTime == 0) {
                printf("Arbitre : timeout depasser Joueur 2 pert la partie\n");
                coupreponse.err = ERR_COUP;
                coupreponse.validCoup = TIMEOUT;
                coupreponse.propCoup = PERDU;
                err = send(Trans1, &coupreponse, sizeof(TCoupRep), 0);
                if (err <= 0) {
                    perror("(serveur) erreur sur le send du code ERR_COUP a Trans1 \n");
                    shutdown(Trans1, SHUT_RDWR);
                    close(Trans1);
                    return -5;
                }
                err = send(Trans2, &coupreponse, sizeof(TCoupRep), 0);
                if (err <= 0) {
                    perror("(serveur) erreur sur le send du code ERR_COUP a Trans2 \n");
                    shutdown(Trans2, SHUT_RDWR);
                    close(Trans2);
                    return -5;
                }

                joueur2Nbperte++;
                joueur1Nbgagne++;
                NbrePartie++;
                break;
            }
        }

        /*
        meme chose pour le time out du premier joueur
        */
        if (theTime == 0) {
            printf("Arbitre : timeout depasser Joueur 1 pert la partie\n");
            coupreponse.err = ERR_COUP;
            coupreponse.validCoup = TIMEOUT;
            coupreponse.propCoup = PERDU;
            err = send(Trans1, &coupreponse, sizeof(TCoupRep), 0);
            if (err <= 0) {
                perror("(serveur) erreur sur le send du code ERR_COUP a Trans1 \n");
                shutdown(Trans1, SHUT_RDWR);
                close(Trans1);
                return -5;
            }
            err = send(Trans2, &coupreponse, sizeof(TCoupRep), 0);
            if (err <= 0) {
                perror("(serveur) erreur sur le send du code ERR_COUP a Trans2 \n");
                shutdown(Trans2, SHUT_RDWR);
                close(Trans2);
                return -5;
            }

            joueur1Nbperte++;
                joueur2Nbgagne++;
            NbrePartie++;
            break;
        }

    }


    /******************************************
    *
    * DEMARAGE DE LA DEUXIEME PARTIE
    *
    *******************************************/


    initialiserPartie();
    printf("Arbitre : PARTIE RETOUR\n");

    while (NbrePartie==2)
    {
        FD_ZERO(&readSet1);
        FD_SET(Trans2, &readSet1);
        struct timeval tval3;
        tval3.tv_sec = TIME_MAX;
        tval3.tv_usec = 0;
        int TheTime1 = select(Trans2 + 1, &readSet1, NULL, NULL, &tval3);

        if (TheTime1 > 0) {
            err = recv(Trans2, &type_requete, sizeof(TIdRequest), MSG_PEEK);
           
            if (err<=0)
            {
                perror("(serveur) erreur sur le recv du code de la requete sur Trans2\n");
                shutdown(Trans2, SHUT_RDWR);
                close(Trans2);
                return -5;
            }
            switch (type_requete)
            {
            case COUP :
                err = recv(Trans2, &couprequete, sizeof(TCoupReq), 0);
                if (err <= 0) {
                    perror("(serveur) erreur sur le recv de la requete sur Trans2\n");
                    shutdown(Trans2, SHUT_RDWR);
                    close(Trans2);
                    return -5;
                }
                printf("(serveur) reception du COUP du joueur 2 \n");
                bool validation2 = validationCoup(1, couprequete, &propCoup);

                if (validation2 == false  ) {
                    coupreponse.err = ERR_COUP;
                    coupreponse.propCoup = PERDU;
                    coupreponse.validCoup = TRICHE;

                    err = send(Trans2, &coupreponse, sizeof(TCoupRep), 0);
                    if (err <= 0) {
                        perror("(serveur) erreur sur le send  de la reponse ERR_COUP sur Trans2\n");
                        shutdown(Trans2, SHUT_RDWR);
                        close(Trans2);
                        return -5;
                    }

                    err = send(Trans1, &coupreponse, sizeof(TCoupRep), 0);
                    if (err <= 0) {
                        perror("(serveur) erreur sur le send  de la reponse ERR_COUP sur Trans1\n");
                        shutdown(Trans1, SHUT_RDWR);
                        close(Trans1);
                        return -5;
                    }
                    joueur1Nbgagne++;
                    joueur2Nbperte++;
                    NbrePartie++;
                    break;
                }
                else {
                    printf("(Arbitre : envoi de la validation du joueur 2 \n");
                    coupreponse.err = ERR_OK;
                    coupreponse.validCoup = VALID;
                    coupreponse.propCoup = propCoup;
                }
                /**/
                err = send(Trans2, &coupreponse, sizeof(TCoupRep), 0);
                if (err <= 0) {
                    perror("(serveur) erreur sur le send  de la reponse de la validation du COUP sur Trans2\n");
                    shutdown(Trans2, SHUT_RDWR);
                    close(Trans2);
                    return -5;
                }
                err = send(Trans1, &coupreponse, sizeof(TCoupRep), 0);
                if (err <= 0) {
                    perror("(serveur) erreur sur le send  de la reponse de la validation du COUP sur Trans1\n");
                    shutdown(Trans1, SHUT_RDWR);
                    close(Trans1);
                    return -5;
                }

                if (validation2 == true && coupreponse.propCoup != CONT) {
                    switch (propCoup)
                    {

                    case GAGNE :
                        joueur2Nbgagne++;
                        joueur1Nbperte++;
                        NbrePartie++;
                        break;
                    case PERDU:
                        joueur1Nbgagne++;
                        joueur1Nbperte++;
                        NbrePartie++;
                        break;
                    case NULLE:
                        joueur1NULLE++;
                        joueur2NULLE++;
                        NbrePartie++;
                        break;

                    default:
                        break;
                    }
                    
                }
                //REMARQUE !
                if (validation2 == true && coupreponse.propCoup == CONT) {
                    printf("ARBITRE  envoi [COUP] a l'adversaire \n");
                    err = send(Trans1, &couprequete, sizeof(TCoupReq), 0);
                        if (err<=0) {
                            perror("(serveur) erreur sur le send de requete sur Trans1\n");
                            shutdown(Trans1, SHUT_RDWR);
                            close(Trans1);
                            return -5;
                        }

                }
                break;


            default:
                coupreponse.err = ERR_TYP;
                err = send(Trans2, &coupreponse, sizeof(TCoupRep), 0); 
                    if (err <= 0) {
                        perror("(serveur) erreur sur le send de reponse avec code ERR_TYP sur Trans2\n");
                        shutdown(Trans2, SHUT_RDWR);
                        close(Trans2);
                        return -5;
                    }
                

                break;
            }

            if (NbrePartie > 2) {
                break;
            }


            /*
            *
            * 
            * 
            */
            printf("Arbitre : attente du COUP du premier joueur...\n");
            FD_ZERO(&readSet1);
            FD_SET(Trans1, &readSet1);
            struct timeval tval4;
            tval4.tv_sec = TIME_MAX;
            tval4.tv_usec = 0;
            int Thetime4 = select(Trans1 + 1, &readSet1, NULL, NULL, &tval4);
            if (Thetime4>0) {
                err = recv(Trans1, &type_requete, sizeof(TIdRequest), MSG_PEEK);
                if (err <= 0) {
                    perror("(serveur) erreur sur le recv du code de requete sur Trans1\n");
                    shutdown(Trans1, SHUT_RDWR);
                    close(Trans1);
                    return -5;
                }

                switch (type_requete) {
                case COUP:
                    err = recv(Trans1, &couprequete, sizeof(TCoupReq), 0);
                    if (err <= 0) {
                        perror("(serveur) erreur sur le recv de la requete [COUP]\n");
                        shutdown(Trans1, SHUT_RDWR);
                        close(Trans1);
                        return -5;
                    }

                    printf("Arbitre :  reception du [COUP] du joueur 1\n");
                    bool validation4 = validationCoup(2, couprequete, &propCoup);

                    if (validation4 == false ) {

                        coupreponse.err = ERR_COUP;
                        coupreponse.validCoup = TRICHE;
                        coupreponse.propCoup = PERDU;
                        err = send(Trans1, &coupreponse, sizeof(TCoupRep), 0);
                        if (err <= 0) {
                            perror("(serveur) erreur sur le send de reponse ERR_COUP sur Trans1\n");
                            shutdown(Trans1, SHUT_RDWR);
                            close(Trans1);
                            return -5;
                        }

                        err = send(Trans2, &coupreponse, sizeof(TCoupRep), 0);
                        if (err <= 0) {
                            perror("(serveur) erreur sur le send de reponse ERR_COUP sur Trans2\n");
                            shutdown(Trans2, SHUT_RDWR);
                            close(Trans2);
                            return -5;
                        }
                        joueur2Nbgagne++;
                        joueur1Nbperte++;
                        NbrePartie++;
                        break;


                    }
                    else {
                        printf("Arbitre : envoi de la validation du COUP ...\n");

                        coupreponse.err = ERR_OK;
                        coupreponse.validCoup = VALID;
                        coupreponse.propCoup = propCoup;
                        }
                        err = send(Trans1, &coupreponse, sizeof(TCoupRep), 0);
                        if (err <= 0) {
                            perror("(serveur) erreur sur le send de reponse VALID sur Trans1\n");
                            shutdown(Trans1, SHUT_RDWR);
                            close(Trans1);
                            return -5;
                        }

                        err = send(Trans2, &coupreponse, sizeof(TCoupRep), 0);
                        if (err <= 0) {
                            perror("(serveur) erreur sur le send de reponse VALID sur Trans2\n");
                            shutdown(Trans2, SHUT_RDWR);
                            close(Trans2);
                            return -5;
                        }

                        if (validation4 == true && propCoup != CONT) {
                            switch (propCoup)
                            {
                            case GAGNE :
                                joueur1Nbgagne++;
                                joueur2Nbperte++;
                                NbrePartie++;
                                break;
                            case PERDU :
                                joueur1Nbperte++;
                                joueur2Nbgagne++;
                                NbrePartie++;
                                break;
                            case NULLE:
                                joueur1NULLE++;
                                joueur2NULLE++;
                                NbrePartie++;
                                break;
                            default:
                                break;
                            }
                        }

                            if (validation4 == true && propCoup == CONT) {
                                printf("Arbitre : envoi du COUP a l'adversaire\n");

                                err = send(Trans2, &couprequete, sizeof(TCoupReq), 0);
                                if (err <= 0) {
                                    perror("(serveur) erreur sur le send de la requte au joueur2\n");
                                    shutdown(Trans2, SHUT_RDWR);
                                    close(Trans2);
                                    return -5;
                                }
                            }
                            break;
       
                default:
                    coupreponse.err = ERR_TYP;
                    err = send(Trans1, &coupreponse, sizeof(TCoupRep), 0);
                    if (err <= 0) {
                        perror("(serveur) erreur sur le send du ERR_COUP sur Trans1\n");
                        shutdown(Trans1, SHUT_RDWR);
                        close(Trans1);
                        return -5;
                    }
                    NbrePartie++;
                    break;
                }

            }


            if (Thetime4 == 0) {
                coupreponse.err = ERR_COUP;
                coupreponse.validCoup = TIMEOUT;
                coupreponse.propCoup = PERDU;
                printf("Arbitre :timeout depasser par le joueur 1 partie PERDU\n ");
                err = send(Trans1, &coupreponse, sizeof(TCoupRep), 0);
                if (err <= 0) {
                    perror("(serveur) erreur sur le send du ERR_COUP sur Trans1\n");
                    shutdown(Trans1, SHUT_RDWR);
                    close(Trans1);
                    return -5;
                }
                err = send(Trans2, &coupreponse, sizeof(TCoupRep), 0);
                if (err <= 0) {
                    perror("(serveur) erreur sur le send du ERR_COUP sur Trans2\n");
                    shutdown(Trans2, SHUT_RDWR);
                    close(Trans2);
                    return -5;
                }
                joueur1Nbperte++;
                joueur2Nbgagne++;
                NbrePartie++;
                    break;


            }

        }

        if (TheTime1 == 0) {
            coupreponse.err = ERR_COUP;
            coupreponse.validCoup = TIMEOUT;
            coupreponse.propCoup = PERDU;
            printf("Arbitre :timeout depasser par le joueur 1 partie PERDU\n ");
            err = send(Trans1, &coupreponse, sizeof(TCoupRep), 0);
            if (err <= 0) {
                perror("(serveur) erreur sur le send du ERR_COUP sur Trans1\n");
                shutdown(Trans1, SHUT_RDWR);
                close(Trans1);
                return -5;
            }
            err = send(Trans2, &coupreponse, sizeof(TCoupRep), 0);
            if (err <= 0) {
                perror("(serveur) erreur sur le send du ERR_COUP sur Trans2\n");
                shutdown(Trans2, SHUT_RDWR);
                close(Trans2);
                return -5;
            }
            joueur1Nbperte++;
            joueur2Nbgagne++;
            NbrePartie++;
                break;


        }
    }

    printf("-----------------------------------------\n");
    printf("| *                                      |\n");
    printf("|                              *         |\n");
    printf("|      *                                 |\n");
    printf("|                RESULTAT  *             |\n");
    printf("|  %s:     partie ganges : %d  et partie perdus : %d et partie nul : %d ", joueur1, joueur1Nbgagne, joueur1Nbperte, joueur1NULLE);
    printf("|  %s:     partie ganges : %d  et partie perdus : %d et partie nul : %d ", joueur2, joueur2Nbgagne, joueur2Nbperte, joueur2NULLE);
    printf("|                                       |\n");
    printf("|    *                                  |\n");
    printf("|                *                      |\n");
    printf("|      *                       *        |\n");
    printf("----------------------------------------\n");



    shutdown(Trans1, SHUT_RDWR);
    close(Trans1);
    shutdown(Trans2, SHUT_RDWR);
    close(Trans2);

    close(sockConx);
    return 0;
}


