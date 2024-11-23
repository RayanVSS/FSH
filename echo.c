#include <unistd.h>
#include <string.h>

/**
 * Fonction echo_command : affiche les arguments donnés avec un espace entre eux.
 * Si l'option -n est fournie, le saut de ligne final est supprimé.
 * Gère également la séquence "\c" pour supprimer le saut de ligne.
 * 
 * @param argc Nombre d'arguments.
 * @param argv Liste des arguments.
 */

void print(char* string , int sortie);

void echo_command(int argc, char *argv[]) {
    int is_n = 1; // Par défaut, ajoute un saut de ligne
    int i = 1;
    int suppress_output = 0; // Gère la suppression de l'affichage après "\c"

    // Vérifie si l'option "-n" est présente
    if (argc > 1 && strcmp(argv[1], "-n") == 0) {
        is_n = 0; // Désactive le saut de ligne final
        i++;      // Passe au premier argument après "-n"
    }

    // Parcourt les arguments
    for (; i < argc; i++) {
        // Si "\c" est rencontré, supprime l'affichage après cet argument
        if (strcmp(argv[i], "\\c") == 0) {
            suppress_output = 1;
            break;
        }

        // Écrit l'argument dans stdout
        print(argv[i], STDOUT_FILENO);
        // Ajoute un espace entre les arguments, sauf après le dernier
        if (i < argc - 1) {
            print(" ", STDOUT_FILENO);
        }
    }

    // Ajoute une nouvelle ligne à la fin, sauf si :
    // - "-n" est utilisé
    // - "\c" a été rencontré
    if (is_n && !suppress_output) {
        print("\n", STDOUT_FILENO);
    }
}
