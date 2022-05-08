#########
#
# Makefile
#
#########

# For Solaris
#LD_FLAGS = -lsocket -lnsl

# For Linux
LD_FLAGS = -g

clean: 
	rm bin/*.o; rm bin/serveurDejeu

all: serveurDejeu

serveurGame: src/serveurGame.c fonctionsTCP 
	gcc src/serveurGame.c -o bin/serveurGame bin/fonctionsTCP.o  $(LD_FLAGS)

fonctionsTCP: fonctions/src/fonctionsTCP.c fonctions/headers/fonctionsTCP.h
	gcc -c -o bin/fonctionsTCP.o fonctions/src/fonctionsTCP.c

fonctionsUDP: fonctions/src/fonctionsUDP.c fonctions/headers/fonctionsUDP.h
	gcc -c -o bin/fonctionsUDP.o fonctions/src/fonctionsUDP.c

