CC = gcc                   
CFLAGS = -Wall -Wextra -g    
LIBS = -lreadline           

# Cible par défaut : créer l'exécutable fsh
all: fsh

fsh: main.o ls.o pwd.o cd.o
	$(CC) $(CFLAGS) -o fsh main.o ls.o pwd.o cd.o $(LIBS)

main.o: main.c
	$(CC) $(CFLAGS) -c main.c

ls.o: ls.c
	$(CC) $(CFLAGS) -c ls.c

pwd.o: pwd.c
	$(CC) $(CFLAGS) -c pwd.c

cd.o: cd.c
	$(CC) $(CFLAGS) -c cd.c

clean:
	rm -f *.o fsh
