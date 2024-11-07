
#include <stdio.h>       
#include <unistd.h>     
#include <limits.h>      
#include <linux/limits.h>

/**
 * Cette fonction affiche le répertoire de travail courant absolu.
 *
 * @return int Retourne 0 en cas de succès, 1 en cas d'échec.
 */
int execute_pwd() {
    char cwd[PATH_MAX]; // Buffer pour stocker le répertoire courant

    // Obtenir le répertoire de travail actuel
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        fprintf(stdout,"%s\n", cwd); // Afficher le répertoire courant
        return 0;            
    } else {
        perror("pwd");       // Afficher une erreur si échoue
        return 1;            
    }
}
