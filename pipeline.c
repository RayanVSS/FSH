#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>

/**
 * Fonction qui crée un pipeline de commandes.
 * 
 * @param cmds Tableau de commandes (chaque commande est un tableau de chaînes).
 * @param n Nombre de commandes dans le pipeline.
 * @param input_file (optionnel) Fichier à utiliser comme entrée standard de la première commande.
 * @param output_file (optionnel) Fichier à utiliser comme sortie standard de la dernière commande.
 * @param error_file (optionnel) Fichier pour rediriger les sorties d'erreur de toutes les commandes.
 * @return int Retourne le code de retour de la dernière commande du pipeline.
 */
int execute_pipeline(char ***cmds, int n, const char *input_file, const char *output_file, const char *error_file) {
    int pipefd[2];
    pid_t pid;
    int in_fd = 0;  // Descripteur d'entrée (initialement stdin)
    int status;
    
    // Si un fichier d'entrée est spécifié, on redirige l'entrée de la première commande
    if (input_file) {
        in_fd = open(input_file, O_RDONLY);
        if (in_fd == -1) {
            perror("Erreur lors de l'ouverture du fichier d'entrée");
            return -1;
        }
    }

    // Si un fichier d'erreur est spécifié, on redirige stderr pour toutes les commandes
    int err_fd = -1;
    if (error_file) {
        err_fd = open(error_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (err_fd == -1) {
            perror("Erreur lors de l'ouverture du fichier d'erreurs");
            return -1;
        }
    }

    for (int i = 0; i < n; i++) {
        // Crée un pipe pour chaque commande sauf la dernière
        if (i < n - 1 && pipe(pipefd) == -1) {
            perror("Erreur lors de la création du pipe");
            return -1;
        }

        pid = fork();
        if (pid == -1) {
            perror("Erreur lors du fork");
            return -1;
        }

        if (pid == 0) {
            // Redirection de l'entrée de la commande
            if (in_fd != 0) {
                dup2(in_fd, STDIN_FILENO);
                close(in_fd);
            }

            // Redirection de la sortie de la commande, sauf pour la dernière
            if (i < n - 1) {
                dup2(pipefd[1], STDOUT_FILENO);
                close(pipefd[0]);
                close(pipefd[1]);
            } else if (output_file) {
                // Redirection de la sortie de la dernière commande vers un fichier si spécifié
                int out_fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (out_fd == -1) {
                    perror("Erreur lors de l'ouverture du fichier de sortie");
                    exit(EXIT_FAILURE);
                }
                dup2(out_fd, STDOUT_FILENO);
                close(out_fd);
            }

            // Redirection de la sortie d'erreur vers un fichier si spécifié
            if (err_fd != -1) {
                dup2(err_fd, STDERR_FILENO);
            }

            // Exécution de la commande
            execvp(cmds[i][0], cmds[i]);
            perror("Erreur lors de l'exécution de la commande");
            exit(EXIT_FAILURE);
        } else {
            // Ferme les descripteurs inutilisés dans le processus parent
            if (in_fd != 0) close(in_fd);
            if (i < n - 1) close(pipefd[1]);

            // L'entrée pour la prochaine commande sera la lecture du pipe actuel
            in_fd = pipefd[0];
        }
    }

    // Ferme le fichier d'erreurs si utilisé
    if (err_fd != -1) close(err_fd);

    // Attendre la fin du dernier processus et récupérer son code de retour
    waitpid(pid, &status, 0);
    return WIFEXITED(status) ? WEXITSTATUS(status) : -1;
}

int main() {
    // Exemple d'utilisation avec `ls | grep .c`
    char *cmd1[] = {"ls", NULL};
    char *cmd2[] = {"grep", ".c", NULL};
    char **cmds[] = {cmd1, cmd2};

    // Appel de la fonction avec redirections optionnelles
    int result = execute_pipeline(cmds, 2, NULL, NULL, "errors.log");
    if (result == -1) {
        fprintf(stderr, "Échec du pipeline\n");
    } else {
        printf("Le pipeline s'est terminé avec le code de retour %d\n", result);
    }

    return result;
}
