#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include "command.h"

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
        close(input_fd); // On peut fermer le fichier après duplication
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