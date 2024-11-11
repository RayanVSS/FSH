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

// Fonction qui donne le nombre de fichier passé en argument
int nb_arguments(char **args) {
    int compt = 0;
    int i = 1;
    while(args[i] != NULL) {
        i++;
        compt++;
    }
    return compt;
}

int executer(char **args,char *path_commande,struct stat path_stat){
    if(stat(path_commande,&path_stat)==0){//Si le fichier existe dans le répertoire courant
        if(path_stat.st_mode & S_IXUSR){//Si le fichier est exécutable 
            char **argv = malloc(100*sizeof(char*));
            argv[0]=path_commande;
            int i = 1;
            while(args[i]!=NULL){
                argv[i]=args[i];
                i++;
            }
            pid_t new_processus=fork();//On cree un processus fils pour exécuter le programme
            if(new_processus==-1){
                fprintf(stderr, "Erreur lors de la création du processus fils\n");
                return 1;
            }

            if(new_processus==0){
                int erreur =execvp(path_commande, argv);//On execute le programme
                perror("execvp");
                return erreur ;exit(1);
            }
            else{
                wait(NULL);
            }
            return 0;
        }
        else{
            print(stdout, strcat("fsh: fichier non exécutable: %s\n",args[0]));
            return 1;
        }
    }
    else {
        return -1;
    }
}



int execute_executable(char **args) {
    struct stat path_stat;
    int verif_execute = executer(args,args[0], path_stat);
    if(verif_execute==0 || verif_execute==1){
        return verif_execute;
    }
    else{ //Si le fichier n'existe pas dans le répertoire courant ,c'est surement une commande externe 
        char *source = getenv("PATH");
        char *p= malloc(strlen(source)+1);
        strcpy(p,source);
        strtok(p, ":");
        char **PATH = malloc(100*sizeof(char*));
        int i = 0;
        while(p != NULL){
            PATH[i++] = p;
            p = strtok(NULL, ":");
        }
        PATH[i] = NULL;
        i = 0;
        while (PATH[i] != NULL) {//On parcourt le PATH
            char *path_commande = concat(concat(PATH[i], "/"), args[0]);
            verif_execute = executer(args,path_commande, path_stat);
            if (verif_execute>=0){
                free(path_commande);
                free(PATH);
                free(p);
                return verif_execute;
            }
            i++;
        }
        if(verif_execute==-1){
            printf("fsh: %s: commande introuvable\n",args[0]);
            free(PATH);
            free(p);
            return 1;
        }
    }

    return 0;
}