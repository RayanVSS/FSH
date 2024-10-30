#include <ncurses.h>
#include <stdlib.h>
#include <string.h>


// Définition de quelques macros pour les touches de contrôle
#define CTRL_KEY(k) ((k) & 0x1f)

// Variables globales pour le nom du fichier et le contenu
char *filename = NULL;
char **lines = NULL;
int num_lines = 0;
int cursor_x = 0, cursor_y = 0;
int screen_rows, screen_cols;

// Initialisation de ncurses
void init_ncurses() {
    initscr();              // Initialise ncurses
    raw();                  // Mode brut pour capturer les touches telles que Ctrl+C
    noecho();               // Ne pas afficher les touches tapées
    keypad(stdscr, TRUE);   // Activer les touches spéciales (flèches, F1, etc.)
    getmaxyx(stdscr, screen_rows, screen_cols); // Obtenir les dimensions de l'écran

    // Désactiver le contrôle de flux pour permettre Ctrl+S et Ctrl+Q
    intrflush(stdscr, FALSE);
    keypad(stdscr, TRUE);
    nonl();
}
// Charger le fichier dans l'éditeur
void load_file(const char *fname) {
    FILE *fp = fopen(fname, "r");
    if (!fp) {
        // Si le fichier n'existe pas, on initialise avec une ligne vide
        num_lines = 1;
        lines = malloc(sizeof(char*));
        lines[0] = strdup("");
        return;
    }

    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    // Lire chaque ligne du fichier
    num_lines = 0;
    while ((read = getline(&line, &len, fp)) != -1) {
        num_lines++;
        lines = realloc(lines, num_lines * sizeof(char*));
        line[strcspn(line, "\n")] = '\0'; // Supprimer le caractère de nouvelle ligne
        lines[num_lines - 1] = strdup(line);
    }
    free(line);
    fclose(fp);
}

// Sauvegarder le fichier
void save_file() {
    if (!filename) return;

    FILE *fp = fopen(filename, "w");
    if (!fp) {
        // Gérer l'erreur d'ouverture du fichier
        return;
    }

    // Écrire chaque ligne dans le fichier
    for (int i = 0; i < num_lines; i++) {
        fprintf(fp, "%s\n", lines[i]);
    }

    fclose(fp);
}

// Afficher le contenu à l'écran
void draw() {
    clear();
    for (int i = 0; i < num_lines && i < screen_rows; i++) {
        mvprintw(i, 0, "%s", lines[i]);
    }
    move(cursor_y, cursor_x);
    refresh();
}

// Gérer les entrées clavier
void process_input() {
    int ch = getch();
    switch (ch) {
        case KEY_UP:
            if (cursor_y > 0) cursor_y--;
            if (cursor_x > strlen(lines[cursor_y])) cursor_x = strlen(lines[cursor_y]);
            break;
        case KEY_DOWN:
            if (cursor_y < num_lines - 1) cursor_y++;
            if (cursor_x > strlen(lines[cursor_y])) cursor_x = strlen(lines[cursor_y]);
            break;
        case KEY_LEFT:
            if (cursor_x > 0) cursor_x--;
            break;
        case KEY_RIGHT:
            if (cursor_x < strlen(lines[cursor_y])) cursor_x++;
            break;
        case KEY_BACKSPACE:
        case 127:
            if (cursor_x > 0) {
                memmove(&lines[cursor_y][cursor_x - 1], &lines[cursor_y][cursor_x], strlen(lines[cursor_y]) - cursor_x + 1);
                cursor_x--;
            } else if (cursor_y > 0) {
                int prev_line_len = strlen(lines[cursor_y - 1]);
                lines[cursor_y - 1] = realloc(lines[cursor_y - 1], prev_line_len + strlen(lines[cursor_y]) + 1);
                strcat(lines[cursor_y - 1], lines[cursor_y]);
                free(lines[cursor_y]);
                memmove(&lines[cursor_y], &lines[cursor_y + 1], (num_lines - cursor_y - 1) * sizeof(char*));
                num_lines--;
                cursor_y--;
                cursor_x = prev_line_len;
            }
            break;
        case '\r':
        case '\n':
            num_lines++;
            lines = realloc(lines, num_lines * sizeof(char*));
            memmove(&lines[cursor_y + 1], &lines[cursor_y], (num_lines - cursor_y - 1) * sizeof(char*));
            lines[cursor_y + 1] = strdup(&lines[cursor_y][cursor_x]);
            lines[cursor_y][cursor_x] = '\0';
            lines[cursor_y] = realloc(lines[cursor_y], cursor_x + 1);
            cursor_y++;
            cursor_x = 0;
            break;
        case CTRL_KEY('s'):
            save_file();
            break;
        case CTRL_KEY('q'):
            endwin();
            exit(0);
            break;
        default:
            lines[cursor_y] = realloc(lines[cursor_y], strlen(lines[cursor_y]) + 2);
            memmove(&lines[cursor_y][cursor_x + 1], &lines[cursor_y][cursor_x], strlen(lines[cursor_y]) - cursor_x + 1);
            lines[cursor_y][cursor_x] = ch;
            cursor_x++;
            break;
    }
}

// Fonction principale
int execute_emacs(int argc, char *argv[]) {
    if (argc >= 2) {
        filename = strdup(argv[1]);
        load_file(filename);
    } else {
        num_lines = 1;
        lines = malloc(sizeof(char*));
        lines[0] = strdup("");
    }

    init_ncurses();

    while (1) {
        draw();
        process_input();
    }

    endwin();
    return 0;
}
