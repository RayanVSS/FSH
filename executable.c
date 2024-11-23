#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>



struct stat *path_stat;
void print(FILE *fd ,char *str);

char *concat(char *s1, char *s2){
    char *result = malloc(strlen(s1)+strlen(s2)+1);
    strcpy(result, s1);
    strcat(result, s2);
    return result;
}

int verif(char *arg){
    char l=arg[0];
    if(l=='<' || l=='>' || l=='|' || l=='&' || l==';'  || l=='{' || l=='}' || l=='$' ||( l=='2' && arg[1]=='>')){
        return 0;
    }

    return 1;
}

// Fonction qui donne le nombre de fichiers passés en argument
int nb_arguments(char **args) {
    int compt = 0;
    int i = 1;
    while(args[i] != NULL) {
        i++;
        compt++;
    }
    return compt;
}

int execute_external_command(char **args) {
    pid_t pid, wpid;
    int status;

    pid = fork(); // Créer un processus enfant

    if (pid == 0) {
        // processus enfant
        signal(SIGINT, SIG_DFL);
        signal(SIGTSTP, SIG_DFL); // Restaurer le comportement par défaut pour Ctrl+Z
        signal(SIGQUIT, SIG_DFL);

        if (execvp(args[0], args) == -1) {
            perror("fsh");
            exit(EXIT_FAILURE);
        }
    } else if (pid < 0) {       
        perror("fsh");
        status = 1; 
    } else {
        // processus parent
        do {
            wpid = waitpid(pid, &status, WUNTRACED);
            if (wpid == -1) {
                perror("fsh");
                status = 1; 
                break;
            }
            if (WIFSTOPPED(status)) {
                const char *msg = "Processus suspendu\n";
                write(STDERR_FILENO, msg, strlen(msg));
                break;
            }
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    if (WIFEXITED(status)) {
        status = WEXITSTATUS(status);
    } else if (WIFSIGNALED(status)) {
        status = 128 + WTERMSIG(status);
    } else if (WIFSTOPPED(status)) {
        status = 148; 
    } else {
        status = 1; 
    }
    return status;
}
