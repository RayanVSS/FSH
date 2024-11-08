#include <stdio.h>
#include <string.h>

/**
 * Fonction echo_command : affiche les arguments donnés avec un espace entre eux.
 * Si l'option -n est fournie, le saut de ligne final est supprimé.
 * 
 * @param argc Nombre d'arguments
 * @param argv Liste des arguments
 * 
 * 
 * ON VA GERER LE -n pour le moment :  -n    Do not print the trailing newline character.  This may also be
           achieved by appending ‘\c’ to the end of the string, as is done by
           iBCS2 compatible systems.  Note that this option as well as the
           effect of ‘\c’ are implementation-defined in IEEE Std 1003.1-2001
           (“POSIX.1”) as amended by Cor. 1-2002.  Applications aiming for
           maximum portability are strongly encouraged to use printf(1) to
           suppress the newline character.
 */

void echo_command(int argc, char *argv[]) {
    int is_n = 1;
    int i = 1;

    // Vérifie si le premier argument est suivant est "-n" et si l

    if (argc > 1 && strcmp(argv[1], "-n") == 0) {
        is_n = 0;  // Si le premier argument après l'appel de la fonction est "-n", on désactive la nouvelle ligne.
        i += 1;         // Les arguments à afficher commencent après "-n".
    }

    // On parcourt les mots
    for (i; i < argc; i++) {
        printf("%s", argv[i]); // Affiche l'argument

        if (i < argc - 1) {
            printf(" ");
        }
    }

    // Ajouter une nouvelle ligne à la fin si is_n est vrai donc si il n'y a pas de -n
    if (is_n) {
        printf("\n");
    }

}