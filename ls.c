#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

// Fonction pour exécuter la commande ls
void execute_ls(char **args) {
    pid_t pid, wpid;
    int status;

    pid = fork();
    if (pid == 0) {
        // Restaurer les signaux par défaut
        signal(SIGINT, SIG_DFL);
        signal(SIGTERM, SIG_DFL);

        // Exécuter ls avec les arguments passés
        if (execvp("ls", args) == -1) {
            perror("fsh");
        }
        exit(EXIT_FAILURE);
    }
    else if (pid < 0) {// Erreur de fork
        perror("fsh");
    }
    else {
        // Parent : attendre que l'enfant termine
        do {
            wpid = waitpid(pid, &status, WUNTRACED);
            if (wpid == -1) {
                perror("waitpid");
                break;
            }
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }
}
