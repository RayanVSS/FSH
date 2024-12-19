// L'idée est de découper en deux fonctions : 
// une premiere qui recupère toutes les informations necessaires
// cad : cmd, la redirection et enfin  le fichier

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h> 
#include "command.h"


#define MAX_ARGS 128
#define MAX_CMD_LENGTH 1024

// plusieurs type de struct etait possible
// après une version prècedente celle la parait plus efficace
/*
typedef struct Command {
    char *name;            // Nom de la commande
    char **args;           // Arguments de la commande
    char *input_file;      // Redirection d'entrée (<)
    char *output_file;     // Redirection de sortie (> ou >>)
    char *error_file;      // Redirection d'erreur (2> ou 2>>)
    int append_output;     // Indicateur pour "ajout" (>> ou 2>>)
} Command;
*/

// Initialisation de la structure Command
Command *init_command() {
    Command *cmd = malloc(sizeof(Command));
    if (!cmd) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    cmd->name = NULL;
    cmd->args = calloc(MAX_ARGS, sizeof(char *));
    if (!cmd->args) {
        perror("calloc");
        free(cmd);
        exit(EXIT_FAILURE);
    }
    cmd->input_file = NULL;
    cmd->output_file = NULL;
    cmd->error_file = NULL;
    cmd->append_output = 0;
    return cmd;
}


// Libération de la mémoire d'une commande
void free_command(Command *cmd) {
    if (!cmd) return;
    if (cmd->name) free(cmd->name);
    if (cmd->args) {
        for (int i = 0; cmd->args[i]; i++) {
            free(cmd->args[i]);
        }
        free(cmd->args);
    }
    if (cmd->input_file) free(cmd->input_file);
    if (cmd->output_file) free(cmd->output_file);
    if (cmd->error_file) free(cmd->error_file);
    free(cmd);
}

// Fonction de parsing pour extraire la commande et les redirections
Command *parse_command(char *input) {
    if (!input) {
        fprintf(stderr, "Erreur : commande vide\n");
        return NULL;
    }

    Command *cmd = init_command();
    char *token = strtok(input, " ");
    int arg_index = 0;

    while (token) {
        if (strcmp(token, "<") == 0) {
            token = strtok(NULL, " ");
            if (!token) {
                fprintf(stderr, "Erreur : fichier manquant après '<'\n");
                free_command(cmd);
                return NULL;
            }
            cmd->input_file = strdup(token);
        } else if (strcmp(token, ">") == 0 || strcmp(token, ">>") == 0 || strcmp(token, ">|") == 0) {
            int append = (strcmp(token, ">>") == 0);
            int force_overwrite = (strcmp(token, ">|") == 0);
            token = strtok(NULL, " ");
            if (!token) {
                fprintf(stderr, "Erreur : fichier manquant après '>' ou '>>' ou '>|'\n");
                free_command(cmd);
                return NULL;
            }
            cmd->output_file = strdup(token);
            cmd->append_output = append;
        } else if (strcmp(token, "2>") == 0 || strcmp(token, "2>>") == 0 || strcmp(token, "2>|") == 0) {
            int append = (strcmp(token, "2>>") == 0);
            int force_overwrite = (strcmp(token, "2>|") == 0);
            token = strtok(NULL, " ");
            if (!token) {
                fprintf(stderr, "Erreur : fichier manquant après '2>' ou '2>>' ou '2>|'\n");
                free_command(cmd);
                return NULL;
            }
            cmd->error_file = strdup(token);
            cmd->append_output = append;
        } else {
            if (arg_index == 0) {
                cmd->name = strdup(token);
            }
            cmd->args[arg_index++] = strdup(token);
        }
        token = strtok(NULL, " ");
    }
    cmd->args[arg_index] = NULL;
    return cmd;
}

// Fonction pour valider et appliquer les redirections
int apply_redirection(Command *cmd) {
    if (!cmd) return -1;

    // Gestion de la redirection d'entrée (<)
    if (cmd->input_file) {
        int input_fd = open(cmd->input_file, O_RDONLY);
        if (input_fd == -1) {
            perror("Erreur ouverture fichier entrée");
            return 1;
        }
        if (dup2(input_fd, STDIN_FILENO) == -1) {
            perror("Erreur duplication entrée");
            close(input_fd);
            return 1;
        }
        close(input_fd);
    }

    // Gestion de la redirection de sortie (> ou >> ou >|)
    if (cmd->output_file) {
        int flags = O_WRONLY | O_CREAT | (cmd->append_output ? O_APPEND : O_TRUNC);
        int output_fd = open(cmd->output_file, flags, 0644);
        if (output_fd == -1) {
            perror("Erreur ouverture fichier sortie");
            return 1;
        }
        if (dup2(output_fd, STDOUT_FILENO) == -1) {
            perror("Erreur duplication sortie");
            close(output_fd);
            return 1;
        }
        close(output_fd);
    }

    // Gestion de la redirection d'erreur (2> ou 2>> ou 2>|)
    if (cmd->error_file) {
        int flags = O_WRONLY | O_CREAT | (cmd->append_output ? O_APPEND : O_TRUNC);
        int error_fd = open(cmd->error_file, flags, 0644);
        if (error_fd == -1) {
            perror("Erreur ouverture fichier erreur");
            return 1;
        }
        if (dup2(error_fd, STDERR_FILENO) == -1) {
            perror("Erreur duplication erreur");
            close(error_fd);
            return 1;
        }
        close(error_fd);
    }

    return 0;
}