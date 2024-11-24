#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/syslimits.h>

int execute_for(char **cmd) {

//Verif du for
    if (cmd == NULL || strcmp(cmd[0], "for") != 0) {
        fprintf(stderr, "Erreur\n");
        return 1;
    }


// On vient vérifier la forme du programme et on extrait

// On verifie la présence de in
    char *premier = cmd[1];


    if (strcmp(cmd[2], "in") != 0) {
        fprintf(stderr, "Veuillez bien formuler le votre for (présence de in)\n");
        return 1;
    }

// De la même façon on vérifie '{' et '}'    
    char *rep = cmd[3];


    if (strcmp(cmd[4], "{") != 0) {
        fprintf(stderr, "Veuillez bien formuler le votre for ( problème '{' )\n");
        return 1;
    }

    // Commande à exécuter pour chaque fichier
    char *cmdd = cmd[5];


    if (cmd[6] == NULL || strcmp(cmd[6], "}") != 0) {
        fprintf(stderr, "Veuillez bien formuler le votre for ( problème '}' )\n");
        return 1;
    }


    DIR *dir = opendir(rep); // cmd[3]
    if (dir == NULL) {
        perror("Erreur : ouverture du répertoire");
        return 1;
    }


struct dirent *entry;
int last_status = 0;

// Parcourir les fichiers du répertoire
while ((entry = readdir(dir)) != NULL) {
    // On saute "." et ".."
    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        continue;

    // Un buffer pour stocker la commande complète
    char full_command[PATH_MAX];
    size_t command_len = strlen(cmdd);

    // Initialiser la commande complète avec la commande de base
    if (command_len >= PATH_MAX) {
        fprintf(stderr, "Erreur : commande trop longue.\n");
        continue;
    }
    strcpy(full_command, cmdd);

    
    if (command_len + 1 >= PATH_MAX) {
        fprintf(stderr, "Erreur : taille de la commande >= a PATH_MAX.\n");
        continue;
    }
    // Ajouter un espace
    strcat(full_command, " ");

    
    if (command_len + 1 + strlen(entry->d_name) >= PATH_MAX) {
        fprintf(stderr, "Erreur : taille de la commande + 1 + strlen(entry->d_name) >= a PATH_MAX\n");
        continue;
    }
    // Ajouter le nom du fichier
    strcat(full_command, entry->d_name);

    // Exécuter la commande pour le fichier
    printf("Exécution de : %s\n", full_command);
    last_status = system(full_command);
}

closedir(dir);
return last_status;
}

