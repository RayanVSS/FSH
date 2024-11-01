#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ncurses.h>
#include <ctype.h>

#define MAX_LINES 1000
#define MAX_LINE_LENGTH 1024

/**
 * @brief Éditeur de texte utilisant ncurses
 *
 * @param args Tableau de chaînes contenant les arguments de la commande `my_editor`.
 * @return int statut
 */
int execute_my_editor(char **args) {
    char *filename = NULL;
    char *lines[MAX_LINES];
    int line_count = 0;
    char buffer[MAX_LINE_LENGTH];
    int ch;

    //tableau des lignes
    for(int i = 0; i < MAX_LINES; i++) {
        lines[i] = NULL;
    }

    //fichier est fourni en argument
    if (args[1] != NULL) {
        filename = strdup(args[1]);
        FILE *fp = fopen(filename, "r");
        if (fp == NULL) {
            perror("Erreur lors de l'ouverture du fichier");
            free(filename);
            filename = NULL;
        } else {
            while (fgets(buffer, MAX_LINE_LENGTH, fp) != NULL && line_count < MAX_LINES) {
                // saut de ligne
                buffer[strcspn(buffer, "\n")] = 0;
                lines[line_count] = strdup(buffer);
                line_count++;
            }
            fclose(fp);
        }
    } else {
        filename = NULL;
    }

    initscr(); //iInitialiser ncurses
    cbreak(); // desactiver la mise en mémoire tampon 
    noecho(); //pas afficher les caractères saisis
    keypad(stdscr, TRUE); // les touches 

    clear();

    mvprintw(0, 0, "=== editeur de Texte  ===");
    mvprintw(1, 0, "Entrez votre texte. Tapez '.save' pour enregistrer et quitter.");
    mvprintw(2, 0, "Tapez '.exit' pour quitter sans enregistrer.");
    move(4, 2);
    refresh();

    int y = 4, x = 2;

    //contenue du fichier
    if (line_count > 0)
        mvprintw(y, x, "le fichier %s contient", (filename != NULL) ? filename : "Nouveau fichier");
    for(int i = 0; i < line_count; i++) {
        y++;
        mvprintw(y, x, "> %s", lines[i]);
    }
    move(y + 1, x);
    refresh();

    while (1) {
        int idx = 0;
        memset(buffer, 0, MAX_LINE_LENGTH);
        y++; 
        mvprintw(y, x - 2, "> ");
        move(y, x);
        refresh();

        while ((ch = getch()) != '\n' && ch != '\r') {
            if (ch == KEY_BACKSPACE || ch == 127) {
                if (idx > 0) {
                    idx--;
                    buffer[idx] = '\0';
                    mvdelch(y, x + idx);
                }
            } else if (isprint(ch) && idx < MAX_LINE_LENGTH - 1) {
                buffer[idx++] = ch;
                addch(ch);
            }
            refresh();
        }

        // terminer la chaîne
        buffer[idx] = '\0';

        //  commandes 
        if (strcmp(buffer, ".save") == 0) {
            if (filename == NULL) {
                echo();
                nocbreak();
                mvprintw(y + 1, 0, "Entrez le nom du fichier pour enregistrer : ");
                char fname[MAX_LINE_LENGTH];
                getnstr(fname, MAX_LINE_LENGTH - 1);
                noecho();
                cbreak();
                filename = strdup(fname);
                y += 2;
                move(y, x);
                refresh();
            }

            // enregistrer dans le fichier
            FILE *fp = fopen(filename, "w");
            if (fp == NULL) {
                mvprintw(y + 1, 0, "Erreur lors de l'ouverture du fichier.\n");
                break;
            }
            for (int i = 0; i < line_count; i++) {
                fprintf(fp, "%s\n", lines[i]);
                free(lines[i]);
                lines[i] = NULL;
            }
            fclose(fp);
            mvprintw(y + 1, 0, "Texte enregistre dans '%s'.\n", filename);
            free(filename);
            break;
        } else if (strcmp(buffer, ".exit") == 0) {
            // quitter sans enregistrer
            for (int i = 0; i < line_count; i++) {
                free(lines[i]);
                lines[i] = NULL;
            }
            free(filename);
            mvprintw(y + 1, 0, "Sortie sans enregistrer.\n");
            break;
        } else {
            if (line_count < MAX_LINES) {
                lines[line_count] = strdup(buffer);
                line_count++;
            } else {
                mvprintw(y + 1, 0, "Nombre maximal de lignes atteint.\n");
            }
        }
    }

    getch();
    endwin();
    return 0;
}

