#include <unistd.h>  
#include <fcntl.h>   
#include <errno.h>   
#include <string.h>  

/**
 * Fonction execute_touch : crée un fichier si celui-ci n'existe pas.
 * 
 * @param argc Nombre d'arguments
 * @param argv Liste des arguments
 * @return int Retourne 0 en cas de succès, 1 en cas d'échec.
 */

int execute_touch(int argc, char *argv[]) {
    // Vérifie s'il y a au moins un fichier en argument
    if (argc < 2) {
        const char *msg = "Mauvaise utilisation, merci de respecter : touch <file1> [<file2> ...]\n";
        write(STDERR_FILENO, msg, strlen(msg));
        return 1;
    }

    for (int i = 1; i < argc; i++) {
        const char *filename = argv[i];

        // Tente de créer le fichier
        int fd = open(filename, O_CREAT | O_EXCL | O_WRONLY, 0644);  // Crée le fichier s'il n'existe pas
        if (fd == -1) {
            // Vérifie si le fichier existe déjà
            if (errno == EEXIST) {
                const char *exist_msg = "Le fichier '";
                const char *exist_end = "' existe déjà, il n'a pas été recréé.\n";
                write(STDOUT_FILENO, exist_msg, strlen(exist_msg));
                write(STDOUT_FILENO, filename, strlen(filename));
                write(STDOUT_FILENO, exist_end, strlen(exist_end));
            } else {
                // Affiche une erreur générique si autre problème
                const char *error_msg = "Erreur lors de la création du fichier : ";
                write(STDERR_FILENO, error_msg, strlen(error_msg));
                write(STDERR_FILENO, filename, strlen(filename));
                write(STDERR_FILENO, "\n", 1);
            }
            continue; // Passe au fichier suivant sans interrompre la boucle
        }

        close(fd); // Ferme le fichier après création
    }

    return 0; // Succès
}
