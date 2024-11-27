#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>


int execute_commande(char **cmd,int last_status) ; 
void print(const char* string , int sortie);

int verif_redirection(char **cmd , int pos){
    if (strcmp(cmd[pos], "<") == 0 || strcmp(cmd[pos], ">") == 0 || strcmp(cmd[pos], "2>") == 0 || strcmp(cmd[pos], ">>") == 0 || strcmp(cmd[pos], "2>>") == 0 || strcmp(cmd[pos], ">|") == 0 || strcmp(cmd[pos], "2>|") == 0){
        return 1;
    }
    return 0;
}

int hasredirection(char** cmd){
    int x=0;
    while (cmd[x]!=NULL){
        if (verif_redirection(cmd,x)==1){
            return x;
        }
        x++;
    }
    return -1;
}

void extract(char ** tokens , char **cmd, int pos) {
    for (int i = 0; i < pos; i++) {
        cmd[i] = tokens[i];
    }
    cmd[pos] = NULL;
}

int execute_redirection (char **tokens , int pos) {
    int flag = 0;
    int sortie_erreur = 0;
    int last_status = 0;

    char **cmd = malloc((pos+1)*sizeof(char *));

    if (cmd == NULL) {
        perror("Erreur lors de l'allocation de mémoire pour les commandes");
        return 1;
    }

    extract(tokens,cmd,pos);

    if (cmd[0]==NULL){
        print("Aucune commande spécifiée\n", STDERR_FILENO);
        return 1;
    } else if(tokens[pos+1]==NULL){
        print("Aucun fichier spécifié\n", STDERR_FILENO);
        return 1;
    }

    if (strcmp(tokens[pos], "<") == 0) {
        flag = O_RDONLY;
        int fd = open(tokens[pos+1],flag);
        if (fd == -1){
            perror("Erreur d'ouverture du fichier");
            return 1;
        }
        int stdin_copy = dup(fileno(stdin));
        dup2(fd,fileno(stdin));
        close(fd);
        
        last_status=execute_commande(cmd,last_status);

        dup2(stdin_copy,fileno(stdin));
        close(stdin_copy);
        return last_status;
    } 
    else{ 
        if (strcmp(tokens[pos], ">") == 0) {
        flag = O_CREAT | O_EXCL | O_WRONLY;
        } else if (strcmp(tokens[pos], "2>") == 0) {
            flag = O_CREAT | O_WRONLY | O_EXCL;
            sortie_erreur = 1;
        } else if (strcmp(tokens[pos], ">>") == 0) {
            flag = O_CREAT | O_WRONLY | O_APPEND;
        } else if (strcmp(tokens[pos], "2>>") == 0) {
            flag = O_CREAT | O_WRONLY | O_APPEND;
            sortie_erreur = 1;
        } else if (strcmp(tokens[pos], ">|") == 0) {
            flag = O_CREAT | O_WRONLY | O_TRUNC;
        } else if (strcmp(tokens[pos], "2>|") == 0) {
            flag = O_CREAT | O_WRONLY | O_TRUNC;
            sortie_erreur = 1;
        }
        else{
            return 1;
        }

        // Sauvegarder les sortie standard et les sorties erreurs 
        int stdout_copy = dup(fileno(stdout));
        int stderr_copy = dup(fileno(stderr));

        int fd = open(tokens[pos+1],flag,0644);
        if (fd == -1){
            perror("Erreur d'ouverture du fichier");
            return 1;
        }

        if (sortie_erreur==1){
            dup2(fd,fileno(stderr));
        }
        else{
            dup2(fd,fileno(stdout));
        }
        close(fd);
        
        // On execute la commande
        last_status=execute_commande(cmd,last_status);

        if (sortie_erreur==1){
            dup2(stderr_copy,fileno(stderr));
            close(stderr_copy);
        }
        else{
            dup2(stdout_copy,fileno(stdout));
            close(stdout_copy);
        }
        return 0;
    }
}