#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <errno.h>
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
        write(STDOUT_FILENO, cwd, strlen(cwd));
        write(STDOUT_FILENO, "\n", 1);
        return 0;            
    } else {
        perror("fsh: pwd");
        return 1;            
    }
}
