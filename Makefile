CC = gcc                   
CFLAGS = -Wall -Wextra -g    
LIBS = -lreadline -lncurses

# Cible par défaut : créer l'exécutable fsh
all: fsh

fsh: main.o ls.o pwd.o cd.o clear.o compgen.o 
	$(CC) $(CFLAGS) -o fsh main.o ls.o pwd.o cd.o clear.o compgen.o $(LIBS)

main.o: main.c
	$(CC) $(CFLAGS) -c main.c

ls.o: ls.c
	$(CC) $(CFLAGS) -c ls.c

pwd.o: pwd.c
	$(CC) $(CFLAGS) -c pwd.c

cd.o: cd.c
	$(CC) $(CFLAGS) -c cd.c

clear.o: clear.c
	$(CC) $(CFLAGS) -c clear.c

man.o: man.c
	$(CC) $(CFLAGS) -c man.c

tree.o: tree.c
	$(CC) $(CFLAGS) -c tree.c
	
open.o: open.c
	$(CC) $(CFLAGS) -c open.c

compgen.o: compgen.c
	$(CC) $(CFLAGS) -c compgen.c
	
myedit.o : myedit.c
	$(CC) $(CFLAGS) -c myedit.c
clean:
	rm -f *.o fsh
