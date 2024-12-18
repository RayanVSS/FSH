#ifndef COMMAND_H
#define COMMAND_H

#define MAX_ARGS 128

typedef struct Command {
    char name;
    char **args;
    charinput_file;
    char output_file;
    charerror_file;
    int append_output;
} Command;

Command init_command();
void free_command(Commandcmd);
Command parse_command(charinput);

#endif // COMMAND_H