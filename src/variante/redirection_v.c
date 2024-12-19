#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include "command.h"

// Applique les redirections d'entrée, de sortie et d'erreur définies dans Command.
int apply_redirection(Command *cmd) {
    if (!cmd) return -1;

    // Sauvegarde les descripteurs d'origine.
    int stdin_copy = dup(STDIN_FILENO);
    int stdout_copy = dup(STDOUT_FILENO);
    int stderr_copy = dup(STDERR_FILENO);
    if (stdin_copy == -1 || stdout_copy == -1 || stderr_copy == -1) {
        perror("dup");
        return 1;
    }

    // Redirection d'entrée (<).
    if (cmd->input_file) {
        int input_fd = open(cmd->input_file, O_RDONLY);
        if (input_fd == -1) {
            perror("Erreur ouverture fichier entrée");
            goto restore;
        }
        if (dup2(input_fd, STDIN_FILENO) == -1) {
            perror("Erreur duplication entrée");
            close(input_fd);
            goto restore;
        }
        close(input_fd);
    }

    // Redirection de sortie (> ou >> ou >|).
    if (cmd->output_file) {
        int flags = O_WRONLY | O_CREAT | (cmd->append_output ? O_APPEND : O_TRUNC);
        int output_fd = open(cmd->output_file, flags, 0644);
        if (output_fd == -1) {
            perror("Erreur ouverture fichier sortie");
            goto restore;
        }
        if (dup2(output_fd, STDOUT_FILENO) == -1) {
            perror("Erreur duplication sortie");
            close(output_fd);
            goto restore;
        }
        close(output_fd);
    }

    // Redirection des erreurs (2> ou 2>> ou 2>|).
    if (cmd->error_file) {
        int flags = O_WRONLY | O_CREAT | (cmd->append_output ? O_APPEND : O_TRUNC);
        int error_fd = open(cmd->error_file, flags, 0644);
        if (error_fd == -1) {
            perror("Erreur ouverture fichier erreur");
            goto restore;
        }
        if (dup2(error_fd, STDERR_FILENO) == -1) {
            perror("Erreur duplication erreur");
            close(error_fd);
            goto restore;
        }
        close(error_fd);
    }

    return 0;

restore:
    // Restaure les descripteurs originaux en cas d'échec.
    dup2(stdin_copy, STDIN_FILENO);
    dup2(stdout_copy, STDOUT_FILENO);
    dup2(stderr_copy, STDERR_FILENO);
    close(stdin_copy);
    close(stdout_copy);
    close(stderr_copy);
    return 1;
}
