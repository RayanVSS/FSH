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
#include <errno.h> // Ajout pour strerror

const char *internal_commands[] = {
     "pwd", "cd", "clear", "history", "exit", "compgen","kill","ftype", NULL
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
int execute_ftype(char **args);

// Fonctions pour gérer les redirections
int verif_redirection(char *x);
int execute_redirection(char **args, int pos, char **commande);

// Fonctions pour gérer les commandes externes
int execute_executable(char **args);
int execute_compgen(const char *internal_commands[], int argc, char **argv);
int execute_external_command(char **args);


// Fonction pour afficher le prompt
void afficher_prompt(int last_status, char *buffer, size_t size) {
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        const char *prefix = "getcwd: ";
        write(STDERR_FILENO, prefix, strlen(prefix));
        write(STDERR_FILENO, strerror(errno), strlen(strerror(errno)));
        write(STDERR_FILENO, "\n", 1);
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

    // tronquer 
    size_t max_length = 27;
    char display_cwd[PATH_MAX];
    if (strlen(cwd) > (max_length - 5)) { // 5 pour "...[x]"
        snprintf(display_cwd, sizeof(display_cwd), "...%s", cwd + strlen(cwd) - (max_length - 5));
    } else {
        strncpy(display_cwd, cwd, sizeof(display_cwd));
        display_cwd[sizeof(display_cwd)-1] = '\0';
    }

    // contruire le prompt
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
            char buffer[1024];
            int len = snprintf(buffer, sizeof(buffer), "%d  %s\n", i + history_base, the_list[i]->line);
            if (len > 0) {
                write(STDOUT_FILENO, buffer, len);
            }
        }
    } else {
        const char *msg = "Aucune commande dans l'historique.\n";
        write(STDERR_FILENO, msg, strlen(msg));
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
    else if (strcmp(cmd[0], "ftype") == 0) {
        last_status = execute_ftype(cmd);
    }
    else {
        last_status = execute_external_command(cmd);
    }
    return last_status;
}



//decouper la ligne en tokens
char **argument(char *line, int *num_tokens) {
    int bufsize = 64;
    int position = 0;
    char **tokens = malloc(bufsize * sizeof(char*));
    if (!tokens) {
        const char *msg = "fsh: allocation error\n";
        write(STDERR_FILENO, msg, strlen(msg));
        exit(EXIT_FAILURE);
    }

    char *token = malloc(strlen(line) + 1);
    if (!token) {
        const char *msg = "fsh: allocation error\n";
        write(STDERR_FILENO, msg, strlen(msg));
        exit(EXIT_FAILURE);
    }
    int tok_pos = 0;
    int in_token = 0;

    for (int i = 0; line[i] != '\0'; i++) {
        if (line[i] == '\\') {
            i++;
            if (line[i] != '\0') {
                token[tok_pos++] = line[i];
            }
        }
        else if (line[i] == ' ') {
            if (in_token) {
                token[tok_pos] = '\0';
                tokens[position++] = strdup(token);
                tok_pos = 0;
                in_token = 0;

                // Réallouer si nécessaire
                if (position >= bufsize) {
                    bufsize += 64;
                    tokens = realloc(tokens, bufsize * sizeof(char*));
                    if (!tokens) {
                        const char *msg = "fsh: allocation error\n";
                        write(STDERR_FILENO, msg, strlen(msg));
                        exit(EXIT_FAILURE);
                    }
                }
            }
        }
        else {
            token[tok_pos++] = line[i];
            in_token = 1;
        }
    }

    if (in_token) {
        token[tok_pos] = '\0';
        tokens[position++] = strdup(token);
    }

    tokens[position] = NULL;
    *num_tokens = position;
    free(token);
    return tokens;
}


int main() {
    char *ligne;
    int last_status = 0;
    char prompt[1024]; // Buffer pour le prompt
    rl_outstream = stderr;


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
            write(STDERR_FILENO, "\n", 1);
            break;
        }

        // Si la ligne n'est pas vide, l'ajouter à l'historique
        if (strlen(ligne) > 0) {
            add_history(ligne);

            // Copier la ligne pour le traitement (strtok modifie la chaîne)
            char *line_copy = strdup(ligne);
            if (!line_copy) {
                const char *prefix = "strdup: ";
                write(STDERR_FILENO, prefix, strlen(prefix));
                write(STDERR_FILENO, strerror(errno), strlen(strerror(errno)));
                write(STDERR_FILENO, "\n", 1);
                free(ligne);
                continue;
            }

            //decouper les tokens
            int num_tokens = 0;
            char **tokens = argument(line_copy, &num_tokens);

            // Exécuter les commandes
            // Boucle pour traiter les tokens

            char **commande = malloc(64*sizeof(char*));
            if(commande==NULL){
                write(STDERR_FILENO, "Allocation error\n", 17);
                free(tokens);
                free(line_copy);
                free(ligne);
                return 1;
            }
            int x=0;
            int y=0;
            while (tokens[x]!=NULL){
                if (verif_redirection(tokens[x])==1){
                    commande[y]=NULL;
                    last_status = execute_redirection(tokens,x,commande);
                    y=0;
                    x++;
                } else if (strcmp(tokens[0], "exit") == 0) { // Comparer avec "exit"
                    int exit_val = (tokens[x+1] != NULL)  ? atoi(tokens[x+1]) : last_status; // Obtenir le code de sortie
                    free(tokens);
                    free(line_copy);
                    free(commande);
                    free(ligne);
                    exit(exit_val);
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
            free(commande);
        }
    
        free(ligne);
    }
    return last_status;

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
                        const char *msg = "fsh: commande non reconnue: ";
                        write(STDERR_FILENO, msg, strlen(msg));
                        write(STDERR_FILENO, tokens[0], strlen(tokens[0]));
                        write(STDERR_FILENO, "\n", 1);
                        last_status = 1; // Erreur générale
                    }
                    }

    */
}   
