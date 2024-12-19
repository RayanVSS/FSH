#ifndef COMMAND_H
#define COMMAND_H

#define MAX_ARGS 128

typedef struct Command {
    char *name;
    char **args;
    char *input_file;
    char *output_file;
    char *error_file;
    int append_output;
} Command;

Command init_command();
void free_command(Command cmd);
Command parse_command(char *input);

#endif // COMMAND_H