#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <limits.h>
#include <linux/limits.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>


const char *internal_commands[] = {
     "pwd", "cd", "clear", "history", "exit", "compgen","kill", NULL
};

//Commandes ici

// Foncctions pour gérer les commandes internes
int execute_pwd();
// void execute_ls(char **args);
int execute_cd(char **args);
void execute_clear(); 
int execute_kill(pid_t pid, int signal);
int execute_history();
// int execute_man(char **args); 
// int execute_cat(char **args);

// Fonctions pour gérer les redirections
int verif_redirection(char x);
int execute_redirection(char **args, int pos, char **commande);

// Fonctions pour gérer les commandes externes
int execute_executable(char **args);
int execute_compgen(const char *internal_commands[], int argc, char **argv);

/*

int execute_external_command(char **args) {
    pid_t pid, wpid;
    int status;

    pid = fork(); // Créer un processus enfant

    if (pid == 0) {
        // processus enfant
        signal(SIGINT, SIG_DFL);
        signal(SIGTERM, SIG_DFL);

        if (execvp(args[0], args) == -1) {
            perror("fsh");
            exit(EXIT_FAILURE);
        }
    } else if (pid < 0) {       
        perror("fsh");
    } else {
        //processus parent
        do {
            wpid = waitpid(pid, &status, WUNTRACED);
            if (wpid == -1) {
                perror("fsh");
                break;
            }
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return status;
}

*/

// Fonction pour print 
void print(FILE *fd ,char *str) {
    write(fileno(fd), str, strlen(str));
}

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


// complétion pour readline
char *init_completion(const char *text, int state) {
    static int list_index, len;
    const char *name;

    if (!state) { // l'index de recherche et long
        list_index = 0;
        len = strlen(text);
    }

    // trouver une correspondance
    while ((name = internal_commands[list_index++])) {
        if (strncmp(name, text, len) == 0) {
            return strdup(name);
        }
    }
    return NULL;
}

//  liste pour readline
char **completion(const char *text, int start, int end) {
    (void)end; //pour le Warn
    // commandes internes
    if (start == 0) {
        return rl_completion_matches(text, init_completion);
    } else {
        // les fichiers
        return rl_completion_matches(text, rl_filename_completion_function);
    }
}


int execute_history() {
    HIST_ENTRY **the_list;
    int i = 0;
    the_list = history_list(); //  liste de historique
    if (the_list && the_list[i+1] != NULL ) {
        for (i = 0; the_list[i+1]; i++) {
            printf("%d  %s\n", i + history_base, the_list[i]->line);
        }
    } else {
        printf("Aucune commande dans l'historique.\n");
        return 1;
    }
    return 0; // Retourne 0 pour indiquer un succès
}

int execute_commande(char **cmd) {
    int last_status = 0;
    if (strcmp(cmd[0], "pwd") == 0) { // Comparer avec "pwd"
        last_status = execute_pwd(); // Appeler execute_pwd et mettre à jour le statut
    }
    else if (strcmp(cmd[0], "cd") == 0) { // Comparer avec "cd"
        last_status = execute_cd(cmd); // Appeler execute_cd et mettre à jour le statut
    }
    else if (strcmp(cmd[0], "clear") == 0) { // Comparer avec "clear"
        execute_clear(cmd); // Appeler execute_clear
        last_status = 0;       // Mettre à jour le statut
    }
    else if (strcmp(cmd[0], "history") == 0) {            
        last_status = execute_history();
    }
    else if (strcmp(cmd[0], "exit") == 0) { // Comparer avec "exit"
        int exit_val = (cmd[0] != NULL) ? atoi(cmd[0]) : last_status; // Obtenir le code de sortie
        exit(exit_val);
    }
    else {
        last_status = execute_executable(cmd);
    }
    return last_status;
}

int main() {
    char *ligne;
    int last_status = 0;
    char prompt[1024]; // Buffer pour le prompt

    // Ignorer SIGINT et SIGTERM dans le shell principal
    signal(SIGINT, SIG_IGN);
    signal(SIGTERM, SIG_IGN);

    rl_attempted_completion_function = completion; // pour Tab

    // Boucle principale du shell
    while (1) {
        // Construire le prompt
        afficher_prompt(last_status, prompt, sizeof(prompt));

        // Lire la ligne de commande avec readline en utilisant le prompt construit
        ligne = readline(prompt);

        if (!ligne) { // EOF (Ctrl-D)
            print(stdout,"\n");
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
                print(stderr, "fsh: allocation error\n");
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
                        print(stderr, "fsh: allocation error\n");
                        free(ligne);
                        free(line_copy);
                        break;
                    }
                }

                token = strtok(NULL, " "); // Obtenir le prochain token
            }
            tokens[position] = NULL; // Terminer le tableau de tokens par NULL

            // Exécuter les commandes
            // Boucle pour traiter les tokens

            char **commande = malloc(64*sizeof(char*));
            if(commande==NULL){
                print(stderr,"Erreur d'allocation de mémoire\n");
                free(tokens);
                free(line_copy);
                free(ligne);
                return 1;
            }
            int x=0;
            int y=0;
            while (tokens[x]!=NULL){
                if (verif_redirection(tokens[x][0])==1){
                    commande[y]=NULL;
                    last_status = execute_redirection(tokens,x,commande);
                    y=0;
                    x++;
                }
                else if (strcmp(tokens[x],"|")==0){
                    // a faire
                    if(commande[0]!=NULL){
                        last_status = execute_commande(commande);
                        y=0;
                    }
                }
                else if (strcmp(tokens[x],";")==0){
                    commande[y]=NULL;
                    if(commande[0]!=NULL){
                        last_status = execute_commande(commande);
                        y=0;
                    }
                }
                else if (strcmp(tokens[x],"&&")==0){
                    commande[y]=NULL;
                    if(commande[0]!=NULL){
                        last_status = execute_commande(commande);
                        y=0;
                    }
                    if(last_status!=0){
                        break;
                    }
                }
                else{
                    commande[y]=tokens[x];
                    if(tokens[x+1]==NULL){
                        commande[y+1]=NULL;
                        last_status = execute_commande(commande);
                    }
                    y++;
                }
                x++;
            }

            free(tokens);
            free(line_copy);
        }
    
        free(ligne);
    }
    return last_status;

    /*
                    else if (strcmp(tokens[0], "kill") == 0) {
                    if (tokens[1] == NULL) {
                        write(STDERR_FILENO, "fsh: kill: manque l'argument du PID\n", 36);
                        last_status = 1;
    */

/*
                else if (strncmp(tokens[0], "./", 2) == 0) {
                    // Essayer d'exécuter une commande externe
                    last_status = execute_external_command(tokens);

                    // Vérifier le statut de retour et ajuster si nécessaire
                    if (WIFEXITED(last_status)) {
                        last_status = WEXITSTATUS(last_status);
                    } else if (WIFSIGNALED(last_status)) {
                        last_status = 128 + WTERMSIG(last_status);
                    } else {
                        last_status = 1; // Erreur générale
                    }
                }
                else {
                    last_status = execute_external_command(tokens);

                // Vérifier le statut de retour et ajuster si nécessaire
                if (WIFEXITED(last_status)) {
                    last_status = WEXITSTATUS(last_status);
                } else if (WIFSIGNALED(last_status)) {
                    last_status = 128 + WTERMSIG(last_status);
                } else {
                    print(stdout, "fsh: commande non reconnue: %s\n", tokens[0]);
                    last_status = 1; // Erreur générale
                }
                }



*/

}   
