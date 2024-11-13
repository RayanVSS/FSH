#include <stdio.h>
#include <sys/stat.h>

/**
 * Cette fonction affiche le type du fichier de référence REF.
 *
 * @param pos Tableau de chaînes de caractères contenant les arguments.
 * @return int Retourne 0 en cas de succès, 1 en cas d'échec.
 */
int execute_ftype(char **args) {
    const char *ref = args[1];
    struct stat st;

    // Vérification du fichier grâce à un lstat
    if (lstat(ref, &st) == -1) {
        perror("ftype:");
        return 1;
    }

    // Affiche le type du fichier de référence REF
    if (S_ISDIR(st.st_mode)) {
        printf("directory\n");
    } else if (S_ISREG(st.st_mode)) {
        printf("regular file\n");
    } else if (S_ISLNK(st.st_mode)) {
        printf("symbolic link\n");
    } else if (S_ISFIFO(st.st_mode)) {
        printf("named pipe\n");
    } else if (S_ISBLK(st.st_mode)) {
        printf("block device\n");
    } else if (S_ISSOCK(st.st_mode)) {
        printf("socket\n");
    } else {
        printf("other\n");
    }

    return 0;
}
