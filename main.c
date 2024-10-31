#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <limits.h>
#include <linux/limits.h>


//Commandes ici
int execute_pwd();
void execute_ls(char **args);
int execute_cd(char **args);
void execute_clear(); 
int execute_man(char **args); 
int execute_cat(char **args, int x);


// Fonction pour afficher le prompt
void afficher_prompt(int last_status, char *buffer, size_t size) {
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("getcwd");
        strcpy(cwd, "?");
    }

    // Couleur en fonction du statut
    char *color = (last_status == 0) ? "\001\033[32m\002" : "\001\033[91m\002"; // Vert ou rouge
    char *reset_color = "\001\033[00m\002"; // Réinitialiser la couleur

    // Format de la valeur de retour
    char status_str[10];
    if (last_status == 255) {
        strcpy(status_str, "SIG");
    } else {
        snprintf(status_str, sizeof(status_str), "%d", last_status);
    }

    // Tronquer si nécessaire
    size_t max_length = 30;
    char display_cwd[PATH_MAX];
    if (strlen(cwd) > (max_length - 5)) { // 5 pour "...[x]"
        snprintf(display_cwd, sizeof(display_cwd), "...%s", cwd + strlen(cwd) - (max_length - 5));
    } else {
        strncpy(display_cwd, cwd, sizeof(display_cwd));
        display_cwd[sizeof(display_cwd)-1] = '\0';
    }

    // Construire le prompt avec les séquences de couleur encapsulées
    snprintf(buffer, size, "%s[%s]%s%s$ ", color, status_str, reset_color, display_cwd);
}

int main() {
    char *ligne;
    int last_status = 0;
    char prompt[1024]; // Buffer pour le prompt

    // Ignorer SIGINT et SIGTERM dans le shell principal
    signal(SIGINT, SIG_IGN);
    signal(SIGTERM, SIG_IGN);

    // Boucle principale du shell
    while (1) {
        // Construire le prompt
        afficher_prompt(last_status, prompt, sizeof(prompt));

        // Lire la ligne de commande avec readline en utilisant le prompt construit
        ligne = readline(prompt);

        if (!ligne) { // EOF (Ctrl-D)
            printf("\n");
            break;
        }

        // Si la ligne n'est pas vide, l'ajouter à l'historique
        if (strlen(ligne) > 0) {
            add_history(ligne);

            // Copier la ligne pour le traitement (strtok modifie la chaîne)
            char *line_copy = strdup(ligne);
            if (!line_copy) {
                perror("strdup");
                free(ligne);
                continue;
            }

            // Initialiser les variables pour le découpage en tokens
            int bufsize = 64, position = 0;
            char **tokens = malloc(bufsize * sizeof(char*)); // Allouer un buffer pour les tokens
            if (!tokens) {
                fprintf(stderr, "fsh: allocation error\n");
                free(ligne);
                free(line_copy);
                continue;
            }

            // Découper la ligne en tokens séparés par des espaces
            char *token = strtok(line_copy, " ");
            while (token != NULL) {
                tokens[position++] = token; // Ajouter le token au tableau

                // Réallouer le buffer si nécessaire
                if (position >= bufsize) {
                    bufsize += 64; // Augmenter la taille du buffer
                    tokens = realloc(tokens, bufsize * sizeof(char*)); // Réallouer
                    if (!tokens) { // Vérifier la réallocation réussie
                        fprintf(stderr, "fsh: allocation error\n");
                        free(ligne);
                        free(line_copy);
                        break;
                    }
                }

                token = strtok(NULL, " "); // Obtenir le prochain token
            }
            tokens[position] = NULL; // Terminer le tableau de tokens par NULL

            // Vérifier si la commande est interne
            if (tokens[0] != NULL) {
                if (strcmp(tokens[0], "ls") == 0) { // Comparer avec "ls"
                    execute_ls(tokens);    // Appeler la fonction execute_ls
                    last_status = 0;       // Mettre à jour le statut (supposé succès)
                }
                else if (strcmp(tokens[0], "pwd") == 0) { // Comparer avec "pwd"
                    last_status = execute_pwd(); // Appeler execute_pwd et mettre à jour le statut
                }
                else if (strcmp(tokens[0], "cd") == 0) { // Comparer avec "cd"
                    last_status = execute_cd(tokens); // Appeler execute_cd et mettre à jour le statut
                }
                else if (strcmp(tokens[0], "clear") == 0) { // Comparer avec "clear"
                    execute_clear(tokens); // Appeler execute_clear
                    last_status = 0;       // Mettre à jour le statut
                }
                else if (strcmp(tokens[0], "man") == 0) { // Comparer avec "man"
                    last_status = execute_man(tokens);
                }
                else if(strcmp(tokens[0],"cat")==0){ // Comparer avec "cat"
                    last_status = execute_cat(tokens,1); // Appeler execute_cat et mettre à jour le statut
                }
                else if (strcmp(tokens[0], "exit") == 0) { // Comparer avec "exit"
                    int exit_val = (tokens[1] != NULL) ? atoi(tokens[1]) : last_status; // Obtenir le code de sortie
                    free(tokens);
                    free(line_copy);
                    free(ligne);
                    exit(exit_val);
                }
                else {
                    // Commande inconnue
                    fprintf(stdout, "fsh: commande non reconnue: %s\n", tokens[0]);
                    last_status = 1; // Mettre à jour le statut en échec
                }
            }

            free(tokens);
            free(line_copy);
        }

        free(ligne);
    }

    return last_status;
}   