#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

int execute_all_commands(char **cmds,int status) ;
char *concat(char *s1, char *s2);
void print(const char* string , int sortie);

int length(char **cmd) {
    int i = 0;
    while (cmd[i] != NULL) {
        i++;
    }
    return i;
}

int *pos_indice(char **cmd, char *indice) {
    int *tab = malloc(sizeof(int) * 50);
    if (tab == NULL) {
        print("fsh: for: Erreur d'allocation\n", STDOUT_FILENO);
        return NULL;
    }
    memset(tab, -1, sizeof(int) * 50);
    int i = 0;
    int j = 0;
    while (cmd[i] != NULL) {
        if (strcmp(cmd[i], indice) == 0) {
            tab[j] = i;
            j++;
        }
        i++;
    }
    return tab;
}

int execute_for(char **cmd) {
    int last_status = 0;

    if (length(cmd) < 6) {
        print("fsh: for: Erreur de syntaxe\n", STDOUT_FILENO);
        return 1;
    }

    char *variable = concat("$", cmd[1]);

    if(strcmp(cmd[2],"in")!=0){
        print("fsh: for: Erreur de syntaxe\n", STDOUT_FILENO);
        return 1;
    }

    char *directory = cmd[3];

    if(strcmp(cmd[4],"{")!=0){
        print("fsh: for: Erreur de syntaxe\n", STDOUT_FILENO);
        return 1;
    }

    char ** commande = malloc(sizeof(char*)*(length(cmd)-4));

    if(commande==NULL){
        print("fsh: for: Erreur d'allocation\n", STDOUT_FILENO);
        return 1;
    }

    int x=5;
    int y=0;

    while(x<length(cmd)-1){
        if(cmd[x]==NULL){
            print("fsh: for: Erreur de syntaxe\n", STDOUT_FILENO);
            free(commande);
            return 1;
        }

        commande[y]=cmd[x];
        x++;
        y++;
    }
    commande[y]=NULL;

    int *indice_variable = pos_indice(commande,variable);
    if(indice_variable==NULL){
        free(commande);
        return 1;
    }


    DIR *dir = opendir(directory);
    if(dir==NULL){
        fprintf(stdout, "fsh: for: %s: Aucun fichier ou dossier de ce type\n",directory);
        free(indice_variable);
        free(commande);
        return 1;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {
            for (int i = 0; indice_variable[i]!=-1 ; i++) {
                commande[indice_variable[i]] = concat(concat(directory, "/"), entry->d_name);
            }
            last_status = execute_all_commands(commande,last_status);
        }
    }

    free(indice_variable);
    free(commande);
    closedir(dir);
    return last_status;

}

