// ls.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <dirent.h>     
#include <sys/stat.h>  
#include <pwd.h>       
#include <grp.h>        
#include <time.h>     

/**
 * Cette fonction liste les fichiers et répertoires dans le répertoire courant ou dans un répertoire spécifié.
 * Elle prend en charge les options suivantes :
 * - `-l` : Affichage détaillé
 * - `-a` : Affichage des fichiers cachés
 *
 * @param args Tableau de chaînes contenant les arguments de la commande `ls`
 */

void execute_ls(char **args,int *pos) {
    DIR *dir;
    struct dirent *entry;
    struct stat file_stat;
    char *path = "."; // Par défaut, répertoire courant
    int long_format = 0;
    int show_all = 0;
    // Parcourir les arguments pour détecter les options et le chemin
    for (int i = *pos; args[i] != NULL; i++) {
        if (args[i][0] == '-') {
            // Parcourir les options après le '-'
            for (size_t j = 1; j < strlen(args[i]); j++) {
                if (args[i][j] == 'l') {
                    long_format = 1;
                }
                else if (args[i][j] == 'a') {
                    show_all = 1;
                }
                else {
                    fprintf(stderr, "fsh: ls: option inconnue -- '%c'\n", args[i][j]);
                    return;
                }
            }
            *pos = *pos + 1; 
        }
        else {
            // Si l'argument ne commence pas par '-', c'est un chemin
            path = args[i];
            *pos = *pos + 1;
        }
    }

    // Ouvrir le répertoire
    dir = opendir(path);
    if (dir == NULL) {
        perror("fsh: ls");
        return;
    }

    // Parcourir les entrées du répertoire
    while ((entry = readdir(dir)) != NULL) {
        // Ignorer les fichiers cachés si l'option '-a' n'est pas spécifiée
        if (!show_all && entry->d_name[0] == '.') {
            continue;
        }

        if (long_format) {
            // Construire le chemin complet
            char full_path[PATH_MAX];
            snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);

            // Obtenir les informations sur le fichier
            if (stat(full_path, &file_stat) == -1) {
                perror("fsh: ls: stat");
                continue;
            }

            // Déterminer le type de fichier
            char file_type;
            if (S_ISREG(file_stat.st_mode)) file_type = '-';
            else if (S_ISDIR(file_stat.st_mode)) file_type = 'd';
            else if (S_ISLNK(file_stat.st_mode)) file_type = 'l';
            else if (S_ISCHR(file_stat.st_mode)) file_type = 'c';
            else if (S_ISBLK(file_stat.st_mode)) file_type = 'b';
            else if (S_ISFIFO(file_stat.st_mode)) file_type = 'p';
            else if (S_ISSOCK(file_stat.st_mode)) file_type = 's';
            else file_type = '?';

            // Permissions
            char permissions[10];
            permissions[0] = (file_stat.st_mode & S_IRUSR) ? 'r' : '-';
            permissions[1] = (file_stat.st_mode & S_IWUSR) ? 'w' : '-';
            permissions[2] = (file_stat.st_mode & S_IXUSR) ? 'x' : '-';
            permissions[3] = (file_stat.st_mode & S_IRGRP) ? 'r' : '-';
            permissions[4] = (file_stat.st_mode & S_IWGRP) ? 'w' : '-';
            permissions[5] = (file_stat.st_mode & S_IXGRP) ? 'x' : '-';
            permissions[6] = (file_stat.st_mode & S_IROTH) ? 'r' : '-';
            permissions[7] = (file_stat.st_mode & S_IWOTH) ? 'w' : '-';
            permissions[8] = (file_stat.st_mode & S_IXOTH) ? 'x' : '-';
            permissions[9] = '\0';

            // Nombre de liens
            int links = file_stat.st_nlink;

            // Propriétaire
            struct passwd *pw = getpwuid(file_stat.st_uid);
            char *owner = pw ? pw->pw_name : "unknown";

            // Groupe
            struct group *gr = getgrgid(file_stat.st_gid);
            char *group = gr ? gr->gr_name : "unknown";

            // Taille
            off_t size = file_stat.st_size;

            // Date de dernière modification
            char time_str[20];
            struct tm *tm_info = localtime(&file_stat.st_mtime);
            strftime(time_str, sizeof(time_str), "%b %d %H:%M", tm_info);

            // Afficher les informations détaillées
            fprintf(stdout,"%c%s %d %s %s %5ld %s %s\n",
                   file_type,
                   permissions,
                   links,
                   owner,
                   group,
                   (long)size,
                   time_str,
                   entry->d_name);
        }
        else {
            // Affichage simple
            fprintf(stdout,"%s  ", entry->d_name);
        }
    }

    if (!long_format) {
        printf("\n");
    }

    closedir(dir);
}
