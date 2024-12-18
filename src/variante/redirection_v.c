#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

// Fonction pour valider les redirections sans utiliser dup2
int validate_redirections(Command cmd) {
    if (!cmd) return -1;

    // Vérification du fichier d'entrée
    if (cmd->input_file) {
        FILEinput = fopen(cmd->input_file, "r");
        if (!input) {
            perror("Erreur ouverture fichier entrée");
            return 1;
        }
        fclose(input);
    }

    // Vérification du fichier de sortie
    if (cmd->output_file) {
        FILE output = fopen(cmd->output_file, cmd->append_output ? "a" : "w");
        if (!output) {
            perror("Erreur ouverture fichier sortie");
            return 1;
        }
        fclose(output);
    }

    // Vérification du fichier d'erreur
    if (cmd->error_file) {
        FILEerror = fopen(cmd->error_file, cmd->append_output ? "a" : "w");
        if (!error) {
            perror("Erreur ouverture fichier erreur");
            return 1;
        }
        fclose(error);
    }

    return 0;
}
