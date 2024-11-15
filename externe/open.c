#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <sys/stat.h>
#include <ctype.h>

typedef struct {
    char *extension;
    char *application;
} ExtensionAppMap;

// tableau de correspondance
ExtensionAppMap mappings[] = {
    { ".txt", "gnome-text-editor" },
    { ".c", "gnome-text-editor" },
    {".py", "gnome-text-editor"},
    { ".md", "gnome-text-editor" },
    { ".sh", "gnome-terminal" },
    { ".pdf", "evince" },
    { ".png", "eog" },
    { ".jpg", "eog" },
    { ".jpeg", "eog" },
    { ".gif", "eog" },
    { ".html", "gnome-www-browser" },
    { ".htm", "gnome-www-browser" },
    { ".doc", "libreoffice" },
    { ".docx", "libreoffice" },
    { ".xls", "libreoffice" },
    { ".xlsx", "libreoffice" },
    { ".ppt", "libreoffice" },
    { ".pptx", "libreoffice" },
    { NULL, NULL } // Marqueur de fin
};

/**
 * @brief Obtient l'application par défaut pour une extension de fichier donnée.
 *
 * @param filename Nom du fichier.
 * @return char* Nom de l'application ou NULL si non trouvé.
 */
char* get_application_for_file(const char *filename) {
    // dernier point dans le nom de fichier
    const char *dot = strrchr(filename, '.');
    if (!dot || dot == filename) return NULL; // Pas d'extension
    // Parcourir le tableau
    for (int i = 0; mappings[i].extension != NULL; i++) {
        if (strcasecmp(dot, mappings[i].extension) == 0) {
            return mappings[i].application;
        }
    }

    return NULL; // Aucune correspondance trouvee
}

/**
 * @brief Vérifie si une chaîne est une URL.
 *
 * @param str Chaîne à vérifier.
 * @return int 1 si c'est une URL, 0 sinon.
 */
int is_url(const char *str) {
    if (str == NULL) return 0;
    if (strncmp(str, "http://", 7) == 0 || strncmp(str, "https://", 8) == 0 ||
        strncmp(str, "ftp://", 6) == 0) {
        return 1;
    }
    return 0;
}

/**
 * @brief Exécute une application pour ouvrir un fichier ou un répertoire.
 *
 * @param args Tableau de chaînes contenant les arguments de la commande `open`.
 * @return int Statut de l'exécution.
 */
int execute_open(char **args) {
    if (args[1] == NULL) {
        fprintf(stderr, "fsh: open: argument manquant\n");
        fprintf(stderr, "Usage: open <fichier_ou_répertoire> [<fichier_ou_répertoire> ...]\n");
        return 1;
    }

    for (int i = 1; args[i] != NULL; i++) {
        char *path = args[i];
        char *app = NULL;

        printf("fsh: open: Traitement de '%s'\n", path);

        // si c'est une URL
        if (is_url(path)) {
            app = "gnome-www-browser"; // Navigateur par défaut
            printf("fsh: open: '%s' est une URL. Application choisie: %s\n", path, app);
        }
        else{   
        //fichier ou répertoire existe
        if (access(path, F_OK) != 0) {
            perror("fsh: open: accès au fichier");
            continue;
        }

        // si c'est un répertoire
        struct stat path_stat;
        if (stat(path, &path_stat) == -1) {
            perror("fsh: open: stat");
            continue;
        }

        if (S_ISDIR(path_stat.st_mode)) {
            // c'est un répertoire
            app = "nautilus";
            printf("fsh: open: '%s' est un répertoire. Application choisie: %s\n", path, app);
        }
        else {
            // obtenir l'application associée
            app = get_application_for_file(path);
            if (app != NULL) {
                printf("fsh: open: Extension trouvée pour '%s'. Application choisie: %s\n", path, app);
            }
            else {
                fprintf(stderr, "fsh: open: pas d'application associée pour '%s'\n", path);
                continue;
            }
        }
        }

        // fork pour exécuter 
        pid_t pid = fork();

        if (pid < 0) {
            perror("fsh: open: fork");
            continue;
        }

        if (pid == 0) {
            // Processus enfant
            signal(SIGINT, SIG_DFL);
            signal(SIGTERM, SIG_DFL);

            // Préparer les arguments pour execvp
            char *exec_args[3];
            exec_args[0] = app;
            exec_args[1] = path;
            exec_args[2] = NULL;

            printf("fsh: open: Exécution de '%s %s'\n", app, path);

            // Exécuter l'application
            if (execvp(app, exec_args) == -1) {
                perror("fsh: open: execvp");
                exit(EXIT_FAILURE);
            }
        }
        else {
            // Processus parent
            printf("fsh: open: processus %d lancé pour ouvrir '%s'\n", pid, path);
        }
    }

    return 0;
}
