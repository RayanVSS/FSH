#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/wait.h>

int execute_man(char **args) {
    if (args[1] == NULL) {
        write(STDERR_FILENO, "fsh: man: aucun argument fourni.\n", 33);
        return 1;
    }

    //fichier temporaire
    int temp_fd = open("temp_man.txt", O_RDWR | O_CREAT | O_TRUNC, 0600);
    if (temp_fd == -1) {
        perror("open");
        return 1;
    }

    // ecrire le contenu du manuel
    const char *content = NULL;
    if (strcmp(args[1], "ls") == 0) {
        content = "LS(1)                    Manuel de fsh                    LS(1)\n\n"
                  "NOM\n       ls - liste les informations sur les fichiers.\n\n"
                  "SYNOPSIS\n       ls [OPTION]... [FICHIER]...\n\n"
                  "-a\n    inclure les entrées débutant par « . »\n\n"
                  "-l\n    utiliser un format d’affichage long\n\n"
                  "DESCRIPTION\n       Liste les informations sur les fichiers (par défaut, le répertoire courant).\n";
    } else if (strcmp(args[1], "pwd") == 0) {
        content = "PWD(1)                   Manuel de fsh                   PWD(1)\n\n"
                  "NOM\n       pwd - affiche le répertoire de travail courant.\n\n"
                  "SYNOPSIS\n       pwd\n\n"
                  "DESCRIPTION\n       Affiche le répertoire de travail courant absolu.\n";
    } else if (strcmp(args[1], "cd") == 0) {
        content = "CD(1)                    Manuel de fsh                    CD(1)\n\n"
                  "NOM\n       cd - change le répertoire de travail courant.\n\n"
                  "SYNOPSIS\n       cd [RÉPERTOIRE]\n\n"
                  "DESCRIPTION\n       Change le répertoire de travail courant vers le répertoire spécifié.\n";
    } else if (strcmp(args[1], "clear") == 0) {
        content = "CLEAR(1)                 Manuel de fsh                 CLEAR(1)\n\n"
                  "NOM\n       clear - efface l'écran du terminal.\n\n"
                  "SYNOPSIS\n       clear\n\n"
                  "DESCRIPTION\n       Efface l'écran du terminal et repositionne le curseur en haut à gauche.\n";
    } else if (strcmp(args[1], "tree") == 0) {
        content = "TREE(1)                 Manuel de fsh                 TREE(1)\n\n"
                  "NOM\n       tree - affiche l'arborescence des répertoires et fichiers.\n\n"
                  "SYNOPSIS\n       tree [OPTION]... [CHEMIN]\n\n"
                  "-a\n    inclure les fichiers cachés\n\n"
                  "-d\n    afficher uniquement les répertoires\n\n"
                  "-L <niveau>\n    limiter l'affichage à une profondeur spécifique\n\n"
                  "-f\n    afficher le chemin complet\n\n"
                  "-h\n    afficher l'aide\n\n"
                  "DESCRIPTION\n       Affiche l'arborescence des répertoires et fichiers à partir du CHEMIN spécifié.\n";
    } else {
        dprintf(STDERR_FILENO, "fsh: man: pas de manuel pour '%s'\n", args[1]);
        close(temp_fd);
        return 1;
    }

    // ecrire dans le fichier 
    if (content) {
        write(temp_fd, content, strlen(content));
    }

    //début pour lecture
    lseek(temp_fd, 0, SEEK_SET);
    
    // afficher
    char buffer[256];
    ssize_t bytes_read;
    int line_count = 0;
    while ((bytes_read = read(temp_fd, buffer, sizeof(buffer))) > 0) {
        for (int i = 0; i < bytes_read; i++) {
            write(STDOUT_FILENO, &buffer[i], 1);
            if (buffer[i] == '\n') {
                line_count++;
            }
        }
    }

    close(temp_fd); 
    unlink("temp_man.txt"); 

    return 0;
}
