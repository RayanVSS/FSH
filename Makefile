CC = gcc                   
CFLAGS = -Wall -Wextra -g    
LIBS = -lreadline -lncurses

# Cible par défaut : créer l'exécutable fsh
all: fsh

fsh: main.o  pwd.o cd.o clear.o  echo.o ftype.o touch.o executable.o compgen.o kill.o redirection.o pipeline.o for.o if.o
	$(CC) $(CFLAGS) -o fsh main.o pwd.o cd.o clear.o echo.o ftype.o touch.o executable.o compgen.o kill.o redirection.o pipeline.o for.o if.o $(LIBS)

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

echo.o: echo.c
	$(CC) $(CFLAGS) -c echo.c

ftype.o: ftype.c
	$(CC) $(CFLAGS) -c ftype.c

touch.o: touch.c  # Ajout de la règle pour compiler touch.c
	$(CC) $(CFLAGS) -c touch.c  # Compilation de touch.c

executable.o: executable.c
	$(CC) $(CFLAGS) -c executable.c
 
tree.o: tree.c
	$(CC) $(CFLAGS) -c tree.c
	
open.o: open.c
	$(CC) $(CFLAGS) -c open.c

compgen.o: compgen.c
	$(CC) $(CFLAGS) -c compgen.c
	
myedit.o : myedit.c
	$(CC) $(CFLAGS) -c myedit.c

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