#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>

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
        type = "directory\n";
    } else if (S_ISREG(st.st_mode)) {
        type = "regular file\n";
    } else if (S_ISLNK(st.st_mode)) {
        type = "symbolic link\n";
    } else if (S_ISFIFO(st.st_mode)) {
        printf("named pipe\n");
    } else if (S_ISBLK(st.st_mode)) {
        printf("block device\n");
    } else if (S_ISSOCK(st.st_mode)) {
        printf("socket\n");
    } else {
        type = "other\n";
    }

    // Écrit le type du fichier dans stdout
    write(STDOUT_FILENO, type, strlen(type));
    return 0;
}
