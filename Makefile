CC = gcc                   
CFLAGS = -Wall -Wextra -g    
LIBS = -lreadline -lncurses

# Cible par défaut : créer l'exécutable fsh
all: fsh

fsh: main.o  pwd.o cd.o clear.o  ftype.o  executable.o kill.o redirection.o pipeline.o for.o if.o
	$(CC) $(CFLAGS) -o fsh main.o pwd.o cd.o clear.o  ftype.o 	 executable.o kill.o redirection.o pipeline.o for.o if.o $(LIBS)

main.o: main.c
	$(CC) $(CFLAGS) -c main.c

pwd.o: pwd.c
	$(CC) $(CFLAGS) -c pwd.c

cd.o: cd.c
	$(CC) $(CFLAGS) -c cd.c

clear.o: clear.c
	$(CC) $(CFLAGS) -c clear.c

man.o: man.c
	$(CC) $(CFLAGS) -c man.c

ftype.o: ftype.c
	$(CC) $(CFLAGS) -c ftype.c

executable.o: executable.c
	$(CC) $(CFLAGS) -c executable.c


kill.o: kill.c
	$(CC) $(CFLAGS) -c kill.c

redirection.o: redirection.c
	$(CC) $(CFLAGS) -c redirection.c

pipeline.o: pipeline.c
	$(CC) $(CFLAGS) -c pipeline.c

if.o: if.c
	$(CC) $(CFLAGS) -c if.c

for.o: for.c
	$(CC) $(CFLAGS) -c for.c

clean:
	rm -f *.o fsh