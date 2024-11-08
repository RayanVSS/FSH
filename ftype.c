#include <stdio.h>
#include <sys/stat.h>

/**
 * Cette fonction affiche le type du fichier de référence REF.
 *
 * @param ref Le chemin vers le fichier de référence.
 * @return int Retourne 0 en cas de succès, 1 en cas d'échec.
 */
int execute_ftype(const char *ref) {
    struct stat st;

    // Vérification du fichier grâce à un lstat
    if (lstat(ref, &st) == -1) {
        perror("Problème avec le fichier");
        return 1;
    }

    // Affiche le type du fichier de référence REF (s'il s'agit d'une référence valide) : directory, regular file , symbolic link, named pipe, other.
    // Après identification grace au if
    if (S_ISDIR(st.st_mode)) {
        printf("directory\n");
    } else if (S_ISREG(st.st_mode)) {
        printf("regular file\n");
    } else if (S_ISLNK(st.st_mode)) {
        printf("symbolic link\n");
    } else if (S_ISFIFO(st.st_mode)) {
        printf("named pipe\n");
    } else {
        printf("other\n");
    }

    return 0;
}
