#include <stdio.h>

/**
 * Cette fonction envoie une séquence d'échappement ANSI pour effacer l'écran du terminal.
 *
 * @param args rien
 */
void execute_clear() {
    printf("\033[2J\033[H");
    fflush(stdout);
}
