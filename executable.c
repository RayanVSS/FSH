#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

/* Ces fonctions permettent soit d'executer un fichier executable dans le repertoire courant
 *ou bien executer une commande externe en parcourant le PATH
 */

struct stat *path_stat;

int verif(char *arg){
    char l=arg[0];
    if(l=='<' || l=='>' || l=='|' || l=='&' || l==';'  || l=='{' || l=='}' || l=='$' ||( l=='2' && arg[1]=='>')){
        return 0;
    }

    return 1;
}

// Fonction qui donne le nombre de fichier passé en argument
int nb_arguments(char **args, int *pos) {
    int compt = 0;
    int i = *pos;
    while(args[i] != NULL) {
        if(verif(args[i])==0) {
           return compt;
        }
        i++;
        compt++;
    }
    return compt;
}

int executer(char **args,char *path_commande, int *pos,struct stat path_stat,int nb){
    if(stat(path_commande,&path_stat)==0){//Si le fichier existe dans le répertoire courant
        if(path_stat.st_mode & S_IXUSR){//Si le fichier est exécutable
            char *commande = path_commande;
            char *arguments[nb+2];
            arguments[0] = commande;
            for(int i=1; i<nb-1; i++){//Ajouter les arguments
                arguments[i] = args[*pos];
                *pos = *pos + 1;
            }
            arguments[nb_arguments(args, pos)+1] = NULL;

            pid_t new_processus=fork();//On cree un processus fils pour exécuter le programme
            if(new_processus==-1){
                fprintf(stderr, "Erreur lors de la création du processus fils\n");
                return 1;
            }

            if(new_processus==0){
                int error = execvp(commande, arguments);//On execute le programme
                if (error == -1) {
                    fprintf(stderr, "Erreur lors de l'exécution du programme\n");
                    return 1;
                }
            }
            else{
                wait(NULL);//On attend la fin de l'exécution du programme
            }
            return 0;
        }
        else{
            fprintf(stdout, "fsh: fichier non exécutable: %s\n", args[*pos-1]);
            *pos = *pos + nb;
            return 1;
        }
    }
    return -1;
}



int execute_executable(char **args, int *pos) {
    struct stat path_stat;
    int nb=nb_arguments(args,pos);
    int verif_execute = executer(args,args[*pos -1], pos, path_stat,nb);
    if(verif_execute==0 || verif_execute==1){
        return verif_execute;
    }
    else{ //Si le fichier n'existe pas dans le répertoire courant ,c'est surement une commande externe 
        char *p= strtok(getenv("PATH"), ":");//On récupère le PATH
        char **PATH = malloc(100*sizeof(char*));
        int i = 0;
        while(p != NULL){
            PATH[i++] = p;
            p = strtok(NULL, ":");
        }
        PATH[i] = NULL;
        i = 0;
        while (PATH[i] != NULL) {//On parcourt le PATH
            char *path_commande = PATH[i];
            strcat(path_commande, "/");
            strcat(path_commande, args[*pos-1]);
            fprintf(stdout, "%s\n", path_commande);
            verif_execute = executer(args,path_commande, pos, path_stat, nb);
            if (verif_execute==0 || verif_execute==1){
                free(path_commande);
                free(PATH);
                free(p);
                return verif_execute;
            }
            i++;
        }
        if(verif_execute==-1){
            fprintf(stdout, "fsh: commande introuvable: %s\n", args[*pos-1]);
            *pos = *pos + nb;
            free(PATH);
            return 1;
        }
    }
    return 0;
}