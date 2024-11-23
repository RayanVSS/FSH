#include <stdio.h>
#include <stdlib.h>

void print(char* string , int sortie);

int execute_cat(char **args) {
    if (strlen(args) < 2) { //Vérifier si le nombre de fichiers est inférieur à 2
        print("cat: Aucun fichier passé en argument \n", stderr);
        return 1;
    }
    int error = 0;
    int i = 1;
    while (args[i]!=NULL){ //Parcourir les fichiers
        FILE *file = fopen(args[i], "r");  //Ouvrir le fichier en mode lecture
        if (file == NULL) { //Vérifier si le fichier est ouvert
            print("Erreur lors de l'ouverture du fichier \n", stderr);
            error = 1;
            continue;
        }

        char buffer[1024];//Créer un buffer pour stocker les données lues
        size_t v_read;//Variable pour stocker le nombre d'octets lus

        while ((v_read = fread(buffer, 1, sizeof(buffer), file)) > 0) {//Lire le fichier par blocs de 1024 octets
            fwrite(buffer, 1, v_read, stdout);//Écrire les données lues sur la sortie standard
        }

        if (ferror(file)) {//Vérifier si une erreur s'est produite lors de la lecture
            print("Erreur lors de la lecture du fichier \n",stderr);
            fclose(file);
            error = 1;
            continue;
        }
        
        fclose(file);
        i++;
    }
    return error;
}