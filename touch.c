#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

/**
 * Fonction execute_touch : crée un fichier si celui-ci n'existe pas.
 * 
 * @param argc Nombre d'arguments
 * @param argv Liste des arguments
 * @return int Retourne 0 en cas de succès, 1 en cas d'échec.
 */

int execute_touch(int argc, char *argv[]) {
    // Y a t'il un fichier ? 
    if (argc < 2) {
        fprintf(stderr, "Mauvaise utilisation, merci de respecter : touch <file1> [<file2> ...]\n");
        return 1;
    }

    for (int i = 1; i < argc; i++) {
        const char *filename = argv[i];

        // Tente de créer le fichier
        int fd = open(filename, O_CREAT | O_EXCL | O_WRONLY, 0644);  // Crée le fichier s'il n'existe pas
        if (fd == -1) {
            // Vérifie si le fichier existe déjà
            if (errno == EEXIST) {
                printf("Le fichier '%s' existe déjà, donc il n'a pas été créé.\n", filename);
            } else {
                perror("Erreur lors de la création du fichier");
            }
            return 1; // Retourne une erreur si une autre raison d'échec
        }

        // Ferme le descripteur de fichier
        close(fd);
        printf("Le fichier '%s' a été créé avec succès.\n", filename);
    }

    return 0; // Succès
}