#include <stdio.h>
#include <stdlib.h>

int execute_cat(char **args) {
    FILE *file = fopen(args[1], "r");  //Ouvrir le fichier en mode lecture
    if (file == NULL) { //Vérifier si le fichier est ouvert
        fprintf(stderr,"Erreur lors de l'ouverture du fichier \n");
        return 1;
    }

    char buffer[1024];//Créer un buffer pour stocker les données lues
    size_t v_read;//Variable pour stocker le nombre d'octets lus

    while ((v_read = fread(buffer, 1, sizeof(buffer), file)) > 0) {//Lire le fichier par blocs de 1024 octets
        fwrite(buffer, 1, v_read, stdout);//Écrire les données lues sur la sortie standard
    }

    if (ferror(file)) {//Vérifier si une erreur s'est produite lors de la lecture
        fprintf(stderr,"Erreur lors de la lecture du fichier \n");
        fclose(file);
        return 1;
    }

    fclose(file);
    return 0;
}