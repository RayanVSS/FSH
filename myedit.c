// my_editor.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


#define MAX_LINES 1000
#define MAX_LINE_LENGTH 1024

/**
 * @brief editeur de texte
 *
 * @param args Tableau de chaines contenant les arguments de la commande `my_editor`.
 * @return int statut
 */
int execute_my_editor(char **args) {
    char *filename = NULL;
    char *lines[MAX_LINES];
    int line_count = 0;
    char buffer[MAX_LINE_LENGTH];

    if (args[1] != NULL) {
        filename = strdup(args[1]);
    } else {
        filename = NULL;
    }

    printf("=== Simple Text Editor ===\n");
    printf("Entrez votre texte. Tapez '.save' pour enregistrer et quitter.\n");
    printf("Tapez '.exit' pour quitter sans enregistrer.\n\n");

    while (1) {
        printf("> ");
        if (fgets(buffer, MAX_LINE_LENGTH, stdin) == NULL) {
            printf("Erreur de lecture.\n");
            break;
        }

        // Supp saut de ligne
        buffer[strcspn(buffer, "\n")] = 0;

        // commandes spéciales
        if (strcmp(buffer, ".save") == 0) {
            if (filename == NULL) {
                printf("Entrez le nom du fichier pour enregistrer : ");
                char fname[MAX_LINE_LENGTH];
                if (fgets(fname, MAX_LINE_LENGTH, stdin) == NULL) {
                    printf("Erreur de lecture du nom de fichier.\n");
                    break;
                }
                fname[strcspn(fname, "\n")] = 0;
                filename = strdup(fname);
            }
            // enregistrer 
            FILE *fp = fopen(filename, "w");
            if (fp == NULL) {
                perror("Erreur lors de l'ouverture du fichier");
                break;
            }
            for (int i = 0; i < line_count; i++) {
                fprintf(fp, "%s\n", lines[i]);
                free(lines[i]); 
            }
            fclose(fp);
            printf("Texte enregistré dans '%s'.\n", filename);
            free(filename);
            return 0;
        } else if (strcmp(buffer, ".exit") == 0) {
            // duitter sans enregistrer
            for (int i = 0; i < line_count; i++) {
                free(lines[i]); 
            }
            free(filename);
            printf("Sortie sans enregistrer.\n");
            return 0;
        } else {
            // ajout la ligne 
            if (line_count < MAX_LINES) {
                lines[line_count] = strdup(buffer);
                line_count++;
            } else {
                printf("Nombre maximal de lignes atteint.\n");
            }
        }
    }

    for (int i = 0; i < line_count; i++) {
        free(lines[i]); 
    }
    free(filename);
    return 0;
}
