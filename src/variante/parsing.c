// L'idée est de découper en deux fonctions : 
// une premiere qui recupère toutes les informations necessaires
// cad : cmd, la redirection et enfin  le fichier

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_ARGS 128
#define MAX_CMD_LENGTH 1024

typedef struct Command {
    char *name;            // Nom de la commande
    char **args;           // Arguments de la commande
    char *input_file;      // Redirection d'entrée (<)
    char *output_file;     // Redirection de sortie (> ou >>)
    char *error_file;      // Redirection d'erreur (2> ou 2>>)
    int append_output;     // Indicateur pour "ajout" (>> ou 2>>)
} Command;

// Initialisation de la structure Command
Command *init_command() {
    Command *cmd = malloc(sizeof(Command));
    if (!cmd) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    cmd->name = NULL;
    cmd->args = malloc(sizeof(char *) * MAX_ARGS);
    if (!cmd->args) {
        perror("malloc");
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

// Fonction de parsing pour les redirections
Command *parse_command(char *input) {
    if (!input) {
        fprintf(stderr, "Erreur \n");
        return NULL;
    }

    Command *cmd = init_command();
    char *token = strtok(input, " ");
    int arg_index = 0;

    while (token) {
        if (strcmp(token, "<") == 0) {
            token = strtok(NULL, " ");
            if (!token) {
                fprintf(stderr, "Erreur : <\n");
                free_command(cmd);
                return NULL;
            }
            cmd->input_file = strdup(token);
        } else if (strcmp(token, ">") == 0 || strcmp(token, ">>") == 0) {
            int append = (strcmp(token, ">>") == 0);
            token = strtok(NULL, " ");
            if (!token) {
                fprintf(stderr, "Erreur > \n");
                free_command(cmd);
                return NULL;
            }
            cmd->output_file = strdup(token);
            cmd->append_output = append;
        } else if (strcmp(token, "2>") == 0 || strcmp(token, "2>>") == 0) {
            int append = (strcmp(token, "2>>") == 0);
            token = strtok(NULL, " ");
            if (!token) {
                fprintf(stderr, "Erreur 2> ou 2>>\n");
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

// Fonction pour valider les fichiers spécifiés dans les redirections
int execute_redirection(Command *cmd) {
    if (!cmd) return -1;

    if (cmd->input_file) {
        FILE *input = fopen(cmd->input_file, "r");
        if (!input) {
            perror("Erreur");
            return 1;
        }
        fclose(input);
    }

    if (cmd->output_file) {
        FILE *output = fopen(cmd->output_file, cmd->append_output ? "a" : "w");
        if (!output) {
            perror("Erreu");
            return 1;
        }
        fclose(output);
    }

    if (cmd->error_file) {
        FILE *error = fopen(cmd->error_file, cmd->append_output ? "a" : "w");
        if (!error) {
            perror("Erreur");
            return 1;
        }
        fclose(error);
    }

    return 0;
}

int main() {
    char input[MAX_CMD_LENGTH];
    printf("Entrez une commande: ");
    if (fgets(input, sizeof(input), stdin) == NULL) {
        perror("fgets");
        return EXIT_FAILURE;
    }

    input[strcspn(input, "\n")] = 0;
    Command *cmd = parse_command(input);
    if (!cmd) return EXIT_FAILURE;

    if (execute_redirection(cmd) != 0) {
        free_command(cmd);
        return EXIT_FAILURE;
    }

    if (cmd->name) {
        printf("Execution de la commande: %s\n", cmd->name);
    }

    free_command(cmd);
    return 0;
}