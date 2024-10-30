#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int execute_man(char **args) {
    if (args[1] == NULL) {
        fprintf(stderr, "fsh: man: aucun argument fourni.\n");
        return 1;
    }

    // Créer un fichier temporaire pour stocker le contenu du manuel
    FILE *temp_file = tmpfile();
    if (temp_file == NULL) {
        perror("tmpfile");
        return 1;
    }

    // fprintf(temp_file, "")
    // Écrire le contenu du manuel dans le fichier temporaire
    if (strcmp(args[1], "ls") == 0) {
        fprintf(temp_file, "LS(1)                    Manuel de fsh                    LS(1)\n\n");
        fprintf(temp_file, "NOM\n       ls - liste les informations sur les fichiers.\n\n");
        fprintf(temp_file, "SYNOPSIS\n       ls [OPTION]... [FICHIER]...\n\n");
        fprintf(temp_file, "-a\n    inclure les entrées débutant par « . »\n\n");
        fprintf(temp_file, "-l\n    utiliser un format d’affichage long\n\n");
        fprintf(temp_file, "DESCRIPTION\n       Liste les informations sur les fichiers (par défaut, le répertoire courant).\n");
    } else if (strcmp(args[1], "pwd") == 0) {
        fprintf(temp_file, "PWD(1)                   Manuel de fsh                   PWD(1)\n\n");
        fprintf(temp_file, "NOM\n       pwd - affiche le répertoire de travail courant.\n\n");
        fprintf(temp_file, "SYNOPSIS\n       pwd\n\n");
        fprintf(temp_file, "DESCRIPTION\n       Affiche le répertoire de travail courant absolu.\n");
    } else if (strcmp(args[1], "cd") == 0) {
        fprintf(temp_file, "CD(1)                    Manuel de fsh                    CD(1)\n\n");
        fprintf(temp_file, "NOM\n       cd - change le répertoire de travail courant.\n\n");
        fprintf(temp_file, "SYNOPSIS\n       cd [RÉPERTOIRE]\n\n");
        fprintf(temp_file, "DESCRIPTION\n       Change le répertoire de travail courant vers le répertoire spécifié.\n");
    } else if (strcmp(args[1], "clear") == 0) {
        fprintf(temp_file, "CLEAR(1)                 Manuel de fsh                 CLEAR(1)\n\n");
        fprintf(temp_file, "NOM\n       clear - efface l'écran du terminal.\n\n");
        fprintf(temp_file, "SYNOPSIS\n       clear\n\n");
        fprintf(temp_file, "DESCRIPTION\n       Efface l'écran du terminal et repositionne le curseur en haut à gauche.\n");
    }else if (strcmp(args[1], "tree")==0){
        fprintf(temp_file, "TREE(1)                 Manuel de fsh                 TREE(1)\n\n");
        fprintf(temp_file, "NOM\n       tree - affiche l'arborescence des répertoires et fichiers.\n\n");
        fprintf(temp_file, "SYNOPSIS\n       tree [OPTION]... [CHEMIN]\n\n");
        fprintf(temp_file, "-a\n    inclure les fichiers cachés\n\n");
        fprintf(temp_file, "-d\n    afficher uniquement les répertoires\n\n");
        fprintf(temp_file, "-L <niveau>\n    limiter l'affichage à une profondeur spécifique\n\n");
        fprintf(temp_file, "-f\n    afficher le chemin complet\n\n");
        fprintf(temp_file, "-h\n    afficher l'aide\n\n");
        fprintf(temp_file, "DESCRIPTION\n       Affiche l'arborescence des répertoires et fichiers à partir du CHEMIN spécifié.\n");
    } else {
        fprintf(stderr, "fsh: man: pas de manuel pour '%s'\n", args[1]);
        fclose(temp_file);
        return 1;
    }

    // revenir en arrire le fichier 
    rewind(temp_file);

    // Ouvrir un pipe
    FILE *less_pipe = popen("less", "w");
    if (less_pipe == NULL) {
        perror("popen");
        fclose(temp_file);
        return 1;
    }

    // Copier vers less
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), temp_file) != NULL) {
        fputs(buffer, less_pipe);
    }

    // Fermer 
    pclose(less_pipe);
    fclose(temp_file);

    return 0;
}
