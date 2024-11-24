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
void print(const char* string , int sortie);

int execute_touch(int argc, char *argv[]) {
    // Vérifie s'il y a au moins un fichier en argument
    if (argc < 2) {
        print("Mauvaise utilisation, merci de respecter : touch <file1> [<file2> ...]\n", STDERR_FILENO);
        return 1;
    }

    for (int i = 1; i < argc; i++) {
        const char *filename = argv[i];

        // Tente de créer le fichier
        int fd = open(filename, O_CREAT | O_EXCL | O_WRONLY, 0644);  // Crée le fichier s'il n'existe pas
        if (fd == -1) {
            // Vérifie si le fichier existe déjà
            if (errno == EEXIST) {
                print("Le fichier '", STDOUT_FILENO);
                print(filename, STDOUT_FILENO);
                print("' existe déjà, il n'a pas été recréé.\n", STDOUT_FILENO);
            } else {
                // Affiche une erreur générique si autre problème
                print("Erreur lors de la création du fichier : ", STDERR_FILENO);
                print(strerror(errno), STDERR_FILENO);
                print("\n", STDERR_FILENO);
            }
            continue; // Passe au fichier suivant sans interrompre la boucle
        }

        close(fd); // Ferme le fichier après création
    }

    return 0; // Succès
}
