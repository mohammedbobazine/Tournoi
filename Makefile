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
	rm bin/*.o; rm bin/serveurGame

all: serveurGame

serveurGame: src/serveurGame.c fonctionsTCP 
	gcc src/serveurGame.c -o bin/serveurGame bin/fonctionsTCP.o bin/colonne.o  $(LD_FLAGS)

serveurGameV2: src/serveurGameV2.c fonctionsTCP 
	gcc src/serveurGameV2.c -o bin/serveurGameV2 bin/fonctionsTCP.o bin/colonne.o  $(LD_FLAGS)

fonctionsTCP: fonctions/src/fonctionsTCP.c fonctions/headers/fonctionsTCP.h
	gcc -c -o bin/fonctionsTCP.o fonctions/src/fonctionsTCP.c


