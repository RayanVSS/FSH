#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

int execute_all_commands(char **cmds,int status);
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
        else {
            char *tmp = cmd[i];
            int k = 0;
            while (tmp[k] != '\0') {
                if (tmp[k] == '$') {
                    if(tmp[k+1]==indice[1]){
                       tab[j] = i;
                       j++;
                       break;
                    }
                }
                k++;
            }
        }
        i++;
    }
    return tab;
}

int nb_occurence(char *c, char *indice) {
    int i = 0;
    int j = 0;
    while (c[i] != '\0') {
        if (c[i] == indice[0] && c[i + 1] == indice[1]) {
                j++;
        }
        i++;
    }
    return j;
}

char * remplace_variable(char * c, char * valeur , char * variable){
    char *tmp = malloc(sizeof(char)*(strlen(c)+(nb_occurence(c,variable))*(strlen(valeur))));
    if(tmp==NULL){
        print("fsh: for: Erreur d'allocation\n", STDOUT_FILENO);
        return NULL;
    }
    int i=0;
    int j=0;
    while(c[i]!='\0'){
        if(c[i]==variable[0] && c[i+1]==variable[1]){
            for(int k=0;valeur[k]!='\0';k++){
                tmp[j]=valeur[k];
                j++;
            }
            i++;
        }
        else{
            tmp[j]=c[i];
            j++;
        }
        i++;
    }
    tmp[j]='\0';
    return tmp;
}

int execute_for(char **cmd) {
    int last_status = 0;
    int parametre[5]={0,0,0,0,0};
    char *extension = NULL;
    char *type = NULL;
    int max_parallel=0;

    if (length(cmd) < 6) {
        print("fsh: for: Erreur de syntaxe\n", STDOUT_FILENO);
        return 1;
    }

    char *variable = concat("$", cmd[1]);

    if(strcmp(cmd[2],"in")!=0){
        print("fsh: for: Erreur de syntaxe\n", STDOUT_FILENO);
        free(variable);
        return 1;
    }

    char *directory = cmd[3];
    int pos = 4;
    while (cmd[pos]!=NULL && strcmp(cmd[pos],"{")!=0) {
        if (strcmp(cmd[pos],"-A")==0) {
            parametre[0]=1;
        }
        else if (strcmp(cmd[pos],"-r")==0) {
            parametre[1]=1;
        }
        else if (strcmp(cmd[pos],"-e")==0) {
            parametre[2]=1;
            pos++;
            if(cmd[pos]==NULL){
                print("fsh: for: Erreur de syntaxe\n", STDOUT_FILENO);
                free(variable);
                return 1;
            }
            extension = malloc(strlen(cmd[pos])+1);
            strcpy(extension,cmd[pos]);
        }
        else if (strcmp(cmd[pos],"-t")==0) {
            parametre[3]=1;
            pos++;
            if(cmd[pos]==NULL){
                print("fsh: for: Erreur de syntaxe\n", STDOUT_FILENO);
                free(variable);
                return 1;
            }
            type = malloc(strlen(cmd[pos])+1);
            strcpy(type,cmd[pos]);
        }
        else if (strcmp(cmd[pos],"-p")==0) {
            parametre[4]=1;
            pos++;
            if(cmd[pos]==NULL){
                print("fsh: for: Erreur de syntaxe\n", STDOUT_FILENO);
                free(variable);
                return 1;
            }
            max_parallel = atoi(cmd[pos]);
        }
        else{
            print("fsh: for: paramêtre inconnu \n", STDOUT_FILENO);
            free(variable);
            return 1;
        }
        pos++;
    }

    if(cmd[pos]==NULL || strcmp(cmd[pos],"{")!=0 || strcmp(cmd[length(cmd)-1],"}")!=0){
        print("fsh: for: Erreur de syntaxe\n", STDOUT_FILENO);
        free(variable);
        if(extension!=NULL) free(extension);
        if(type!=NULL) free(type);
        return 1;
    }

    pos+=1;

    char ** commande = malloc(sizeof(char*)*(length(cmd)-4));
    if(commande==NULL){
        print("fsh: for: Erreur d'allocation\n", STDOUT_FILENO);
        free(variable);
        if(extension!=NULL) free(extension);
        if(type!=NULL) free(type);
        return 1;
    }
    int y=0;
    int sauvegarde = pos;

    while(pos<length(cmd)-1){
        if(cmd[pos]==NULL){
            print("fsh: for: Erreur de syntaxe\n", STDOUT_FILENO);
            free(commande);
            free(variable);
            if(extension!=NULL) free(extension);
            if(type!=NULL) free(type);
            return 1;
        }
        commande[y] = malloc(sizeof(char)*(strlen(cmd[pos])+1));
        strcpy(commande[y],cmd[pos]);
        pos++;
        y++;
    }
    commande[y]=NULL;

    int *indice_variable = pos_indice(commande,variable);
    if(indice_variable==NULL){
        free(commande);
        free(variable);
        if(extension!=NULL) free(extension);
        if(type!=NULL) free(type);
        return 1;
    }

    DIR *dir = opendir(directory);
    if(dir==NULL){
        fprintf(stdout, "fsh: for: %s: Aucun fichier ou dossier de ce type\n",directory);
        free(indice_variable);
        for(int i=0; commande[i]!=NULL; i++){
            free(commande[i]);
        }
        free(commande);
        free(variable);
        if(extension!=NULL) free(extension);
        if(type!=NULL) free(type);
        return 1;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if ((parametre[0]==1 || entry->d_name[0]!='.')) {
            if(strcmp(entry->d_name,".")==0 || strcmp(entry->d_name,"..")==0) continue;
            if(parametre[2]==1){
                int len = strlen(entry->d_name);
                int elen = strlen(extension);
                if(!(len>elen && strcmp(entry->d_name+len-elen,extension)==0)){
                    continue;
                }
            }
            if(parametre[3]==1){
                struct stat st;
                char *p = concat(concat(directory,"/"),entry->d_name);
                if(lstat(p,&st)==-1){
                    free(p);
                    continue;
                }
                if(type[0]=='f' && !S_ISREG(st.st_mode)){
                    free(p);
                    continue;
                }
                if(type[0]=='d' && !S_ISDIR(st.st_mode)){
                    free(p);
                    continue;
                }
                if(type[0]=='l' && !S_ISLNK(st.st_mode)){
                    free(p);
                    continue;
                }
                if(type[0]=='p' && !S_ISFIFO(st.st_mode)){
                    free(p);
                    continue;
                }
                free(p);
            }

            for (int i = 0; indice_variable[i]!=-1 ; i++) {
                char *val = concat(concat(directory, "/"), entry->d_name);
                if(parametre[2]==1){
                    int len = strlen(val);
                    int elen = strlen(extension);
                    val[len-elen-1]='\0';
                }
                if(strcmp(variable, commande[indice_variable[i]])==0){
                    free(commande[indice_variable[i]]);
                    commande[indice_variable[i]] = val;
                }
                else{
                    char *tmp=remplace_variable(cmd[indice_variable[i]+sauvegarde],val,variable);
                    free(commande[indice_variable[i]]);
                    free(val);
                    commande[indice_variable[i]]=tmp;
                }
            }

            int status = execute_all_commands(commande,last_status);
            if(status>last_status){
                last_status = status;
            }
        }
    }

    free(indice_variable);
    for(int i=0; commande[i]!=NULL; i++){
        free(commande[i]);
    }
    free(commande);
    free(variable);
    if(extension!=NULL) free(extension);
    if(type!=NULL) free(type);
    closedir(dir);
    return last_status;
}
