#include <stdio.h>
#include <stdlib.h>

// Fonction qui donne le nombre de fichier passé en argument
int nb_fichier(char **args, int *pos) {
    int compt = 0;
    int i = *pos;
    while(args[i] != NULL) {
        if(!(args[i][0] <= 'z' && args[i][0] >= 'a') && !(args[i][0] <= 'Z' && args[i][0] >= 'A')) {
           return compt;
        }
        i++;
        compt++;
    }
    return compt;
}

int execute_cat(char **args, int* pos ) {
    int nb=nb_fichier(args,pos);
    if (nb < 1) { //Vérifier si le nombre de fichiers est inférieur à 2
        fprintf(stderr, "cat: Aucun fichier passé en argument \n");
        return 1;
    }
    int error = 0;
    for (int i = 0; i < nb; i++) { //Parcourir les fichiers
        FILE *file = fopen(args[*pos], "r");  //Ouvrir le fichier en mode lecture
        if (file == NULL) { //Vérifier si le fichier est ouvert
            fprintf(stderr,"Erreur lors de l'ouverture du fichier \n");
            error = 1;
            continue;
        }

        char buffer[1024];//Créer un buffer pour stocker les données lues
        size_t v_read;//Variable pour stocker le nombre d'octets lus

        while ((v_read = fread(buffer, 1, sizeof(buffer), file)) > 0) {//Lire le fichier par blocs de 1024 octets
            fwrite(buffer, 1, v_read, stdout);//Écrire les données lues sur la sortie standard
        }

        if (ferror(file)) {//Vérifier si une erreur s'est produite lors de la lecture
            fprintf(stderr,"Erreur lors de la lecture du fichier \n");
            fclose(file);
            error = 1;
            continue;
        }
        
        fclose(file);
        *pos = *pos + 1;
    }
    return error;
}