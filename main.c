#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <limits.h>

//mettre les commandes ici
int execute_pwd();
void execute_ls(char **args);
int execute_cd(char **args);


// Fonction pour afficher le prompt
void afficher_prompt(int last_status) {
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("getcwd");
        strcpy(cwd, "?");
    }

    //couleur en fonction du statut
    char *color = (last_status == 0) ? "\001\033[32m\002" : "\001\033[91m\002";
    char *reset_color = "\001\033[00m\002";

    // Format de la valeur de retour
    char status_str[10];
    if (last_status == 255) {
        strcpy(status_str, "SIG");
    } else {
        snprintf(status_str, sizeof(status_str), "%d", last_status);
    }

    // Tronquer si nécessaire
    int max_length = 30;
    char display_cwd[PATH_MAX];
    if (strlen(cwd) > max_length - 5) { // 5 pour "[x]..."
        snprintf(display_cwd, sizeof(display_cwd), "...%s", cwd + strlen(cwd) - (max_length - 5));
    } else {
        strncpy(display_cwd, cwd, sizeof(display_cwd));
        display_cwd[sizeof(display_cwd)-1] = '\0';
    }

    // Construire le prompt
    fprintf(stderr, "%s[%s]%s%s$ ", color, status_str, reset_color, display_cwd);
}

int main() {
    char *ligne;
    int last_status = 0;

    // Ignorer SIGINT et SIGTERM dans le shell principal
    signal(SIGINT, SIG_IGN);
    signal(SIGTERM, SIG_IGN);

    while (1) {
        afficher_prompt(last_status);

        // Lire la ligne de commande avec readline
        ligne = readline(NULL);

        if (!ligne) { // EOF (Ctrl-D)
            printf("\n");
            break;
        }

        // Si la ligne n'est pas vide, l'ajouter à l'historique
        if (strlen(ligne) > 0) {
            add_history(ligne);

            // Copier la ligne pour le traitement
            char *line_copy = strdup(ligne);
            if (!line_copy) {
                perror("strdup");
                free(ligne);
                continue;
            }

            // Découper la ligne en tokens
            int bufsize = 64, position = 0;
            char **tokens = malloc(bufsize * sizeof(char*));
            if (!tokens) {
                fprintf(stderr, "fsh: allocation error\n");
                free(ligne);
                free(line_copy);
                continue;
            }

            char *token = strtok(line_copy, " ");
            while (token != NULL) {
                tokens[position++] = token;

                if (position >= bufsize) {
                    bufsize += 64;
                    tokens = realloc(tokens, bufsize * sizeof(char*));
                    if (!tokens) {
                        fprintf(stderr, "fsh: allocation error\n");
                        free(ligne);
                        free(line_copy);
                        break;
                    }
                }

                token = strtok(NULL, " ");
            }
            tokens[position] = NULL;

            if (tokens[0] != NULL) {
                //Commande "ls"
                if (strcmp(tokens[0], "ls") == 0) {
                    execute_ls(tokens);
                    last_status = 0; // Supposons qu'il réussit
                }
                //Commande "pwd"
                else if (strcmp(tokens[0], "pwd") == 0) {
                    last_status = execute_pwd();
                }
                // Commande "cd"
                else if (strcmp(tokens[0], "cd") == 0) {
                    last_status = execute_cd(tokens);
                }
                
                //pour quiter le terminal 
                else if (strcmp(tokens[0], "exit") == 0) {
                    int exit_val = (tokens[1] != NULL) ? atoi(tokens[1]) : last_status;
                    free(tokens);
                    free(line_copy);
                    free(ligne);
                    exit(exit_val);
                }
                else {
                    // Commande inconnue
                    fprintf(stderr, "fsh: commande non reconnue: %s\n", tokens[0]);
                    last_status = 1;
                }
            }

            free(tokens);
            free(line_copy);
        }

        free(ligne);
    }

    return last_status;
}
