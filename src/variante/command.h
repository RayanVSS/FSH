#ifndef COMMAND_H
#define COMMAND_H

// Ce fichier définit la structure Command, qui sert à représenter une commande shell avec ses arguments,
// ses redirections d'entrée (<), de sortie (>) et d'erreur (2>).

#define MAX_ARGS 128

typedef struct Command {
    char *name;
    char **args;
    char *input_file;
    char *output_file;
    char *error_file;
    int append_output;
} Command;

// Initialise Command.
Command *init_command();

// Libère la mémoire d'une struct Command.
void free_command(Command *cmd);

// extrait les eléments necessaire
Command *parse_command(const char *input);

#endif // COMMAND_H
