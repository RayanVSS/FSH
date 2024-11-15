#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <unistd.h>
#include <linux/limits.h>


// couleur ANSI
#define COLOR_RESET "\033[0m"
#define COLOR_DIR   "\033[34m" 
#define COLOR_EXEC  "\033[32m" 
#define COLOR_LINK  "\033[36m" 
#define COLOR_FIFO  "\033[33m"
#define COLOR_SOCK  "\033[35m" 
#define COLOR_BLK   "\033[35m"
#define COLOR_CHR   "\033[35m" 

//les options
typedef struct {
    int all;              //  -a
    int directories_only; //  -d
    int max_depth;        //  -L
    int full_path;        //  -f
} tree_options;

// répéter un caractère n fois
void print_indent(int depth, int last[]) {
    for(int i = 0; i < depth; i++) {
        if (last[i])
            printf("    ");
        else
            printf("│   ");
    }
}

//répertoire est accessible
int is_accessible(const char *path) {
    struct stat info;
    if(stat(path, &info) != 0)
        return 0;
    return S_ISDIR(info.st_mode);
}

//code de couleur 
const char* get_color(const struct stat *st) {
    if (S_ISDIR(st->st_mode)) {return COLOR_DIR;}
    else if (S_ISLNK(st->st_mode)) {return COLOR_LINK;}
    else if (S_ISFIFO(st->st_mode)) {return COLOR_FIFO;}
    else if (S_ISSOCK(st->st_mode)) {return COLOR_SOCK;}
    else if (S_ISBLK(st->st_mode)) {return COLOR_BLK;}
    else if (S_ISCHR(st->st_mode)) {return COLOR_CHR;}
    else if (st->st_mode & S_IXUSR) { return COLOR_EXEC;}
    else {return COLOR_RESET;}
}

//cible d'un lien symbolique
void print_symlink(const char *path) {
    char target[PATH_MAX];
    ssize_t len = readlink(path, target, sizeof(target) - 1);
    if (len != -1) {
        target[len] = '\0';
        printf(" -> %s", target);
    }
}

// parcourir les répertoires
void tree(const char *path, int depth, int last[], tree_options options) {
    if (depth >= 100) {
        fprintf(stderr, "Erreur: Profondeur maximale dépassée pour %s\n", path);
        return;
    }

    //profondeur maximale 
    if (options.max_depth != -1 && depth > options.max_depth) {
        return;
    }

    DIR *dir;
    struct dirent *entry;
    struct stat st;
    char fullpath[PATH_MAX];
    size_t count = 0;


    if (!(dir = opendir(path))) {
        fprintf(stderr, "Erreur: Impossible d'ouvrir le répertoire %s: %s\n", path, strerror(errno));
        return;
    }

    // Compter le nombre d'entrees 
    while ((entry = readdir(dir)) != NULL) {
        //ignorer les fichiers cachés (sauf -a)
        if (!options.all && entry->d_name[0] == '.')
            continue;
        if (strcmp(entry->d_name, ".") && strcmp(entry->d_name, ".."))
            count++;
    }
    rewinddir(dir);

    size_t current = 0;
    //parcourir les entrees 
    while ((entry = readdir(dir)) != NULL) {
        if (!options.all && entry->d_name[0] == '.')
            continue;
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        current++;
        // chemin complet
        if (snprintf(fullpath, sizeof(fullpath), "%s/%s", path, entry->d_name) >= (int)sizeof(fullpath)) {
            fprintf(stderr, "Erreur: Chemin trop long pour %s/%s\n", path, entry->d_name);
            continue;
        }

        // informations sur le fichier
        if (lstat(fullpath, &st) == -1) {
            fprintf(stderr, "Erreur: Impossible de stat %s: %s\n", fullpath, strerror(errno));
            continue;
        }

        // -d
        if (options.directories_only && !S_ISDIR(st.st_mode))
            continue;

        //indentation
        print_indent(depth, last);

        //  les branches
        if (current == count)
            printf("└── ");
        else
            printf("├── ");

        //couleur
        const char *color = get_color(&st);

        if (options.full_path) {
            printf("%s%s%s", color, fullpath, COLOR_RESET);
        } else {
            printf("%s%s%s", color, entry->d_name, COLOR_RESET);
        }

        //cible si c'est un lien symbolique
        if (S_ISLNK(st.st_mode)) {
            print_symlink(fullpath);
        }

        printf("\n");

        // Si répertoire
        if (S_ISDIR(st.st_mode)) {
            if (current == count)
                last[depth] = 1;
            else
                last[depth] = 0;
            tree(fullpath, depth + 1, last, options);
        }
    }

    closedir(dir);
}

// Fonction pour afficher un message d'aide
void print_help() {
    printf("Usage: tree [OPTIONS] [CHEMIN]\n");
    printf("Options:\n");
    printf("  -a            Inclure les fichiers cachés\n");
    printf("  -d            Afficher uniquement les répertoires\n");
    printf("  -L <niveau>   Limiter l'affichage à une profondeur spécifique\n");
    printf("  -f            Afficher le chemin complet\n");
    printf("  -h            Afficher l'aide\n");
}

// Fonction pour exécuter la commande tree avec les options
int execute_tree(int argc, char *argv[]) {
    tree_options options = {0, 0, -1, 0}; // Initialiser les options par défaut (max_depth = -1)
    char *path = NULL;

    // Parcourir les arguments pour détecter les options et le chemin
    for (int i = 1; i < argc; i++) {
        // Vérifier si l'argument commence par '-'
        if (argv[i][0] == '-') {
            // Parcourir chaque caractère de l'option (sauf le premier '-')
            for (size_t j = 1; j < strlen(argv[i]); j++) {
                char opt = argv[i][j];
                switch (opt) {
                    case 'a':
                        options.all = 1;
                        break;
                    case 'd':
                        options.directories_only = 1;
                        break;
                    case 'L':
                        // Vérifier si l’option 'L' a un argument attaché (e.g., -L3)
                        if (argv[i][j + 1] != '\0') {
                            options.max_depth = atoi(&argv[i][j + 1]);
                            j = strlen(argv[i]) - 1; // Fin de cette option
                        }
                        // Sinon, prendre l’argument suivant (e.g., -L 3)
                        else if (i + 1 < argc) {
                            options.max_depth = atoi(argv[++i]);
                        } else {
                            fprintf(stderr, "Erreur: L'option -L nécessite un argument.\n");
                            print_help();
                            return EXIT_FAILURE;
                        }

                        if (options.max_depth < 0) {
                            fprintf(stderr, "Erreur: Niveau de profondeur invalide: %d\n", options.max_depth);
                            return EXIT_FAILURE;
                        }
                        break;
                    case 'f':
                        options.full_path = 1;
                        break;
                    case 'h':
                        print_help();
                        return EXIT_SUCCESS;
                    default:
                        fprintf(stderr, "Erreur: Option inconnue -- '%c'\n", opt);
                        print_help();
                        return EXIT_FAILURE;
                }
            }
        }
        else {
            // Si l'argument ne commence pas par '-', c'est un chemin
            if (path == NULL) { // Prendre le premier chemin fourni
                path = argv[i];
            }
            else {
                fprintf(stderr, "Erreur: Plusieurs chemins fournis. Veuillez spécifier un seul chemin.\n");
                print_help();
                return EXIT_FAILURE;
            }
        }
    }

    // Définir le chemin par défaut si aucun chemin n'est fourni
    if (path == NULL) {
        path = ".";
    }

    // Vérifier si le chemin est accessible
    if (!is_accessible(path)) {
        fprintf(stderr, "Erreur: %s n'est pas un répertoire accessible.\n", path);
        return EXIT_FAILURE;
    }

    printf("%s\n", options.full_path ? path : ".");
    int last[100] = {0}; // Tableau pour gérer les lignes d'indentation
    tree(path, 0, last, options);

    return EXIT_SUCCESS;
}

