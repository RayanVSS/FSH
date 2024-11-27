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
     "pwd", "cd", "clear", "history", "exit","kill", "ftype", NULL
};

// Foncctions pour gérer les commandes internes
int execute_pwd();
int execute_cd(char **args);
void execute_clear(); 
int execute_history();
int execute_ftype(char **args);
int execute_kill(char ** args);

// pour if
int execute_if(char **cmd);

// Fonctions pour gérer les redirections
int hasredirection(char** cmd);
int execute_redirection(char **args, int pos);

// Fonctions pour gérer les pipelines
int execute_pipeline(char **commande, int pipeline);
int haspipeline(char **cmd);

// Fonctions pour gérer les for 
int execute_for(char **cmd);

// Fonctions pour gérer les commandes externes
int execute_executable(char **args);
int execute_external_command(char **args);


void print(const char* string , int sortie){
    write(sortie,string,strlen(string));
    
}

// Fonction pour afficher le prompt
void afficher_prompt(int last_status, char *buffer, size_t size) {
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        const char *prefix = "getcwd: ";
        print(prefix,STDERR_FILENO);
        print(strerror(errno),STDERR_FILENO);
        print("\n",STDERR_FILENO);
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
                print(buffer, STDOUT_FILENO);
            }
        }
    } else {
        print("Aucune commande dans l'historique.\n", STDOUT_FILENO);
        return 1;
    }
    return 0; // Retourne 0 pour indiquer un succès
}

int execute_commande(char **cmd, int status) {
    int last_status = status;
    int redirection = hasredirection(cmd);
    int pipeline = haspipeline(cmd);
    if (strcmp(cmd[0],"for")==0){
        last_status = execute_for(cmd);
    }
    else if (strcmp(cmd[0], "if") == 0) { 
        last_status = execute_if(cmd);
    }
    else if (pipeline != 0) {
        last_status = execute_pipeline(cmd, pipeline+1);
    }
    else if (redirection != -1) {
        last_status = execute_redirection(cmd, redirection);
    }
    else if (strcmp(cmd[0], "pwd") == 0) { 
        last_status = execute_pwd(); 
    }
    else if (strcmp(cmd[0], "cd") == 0) { 
        last_status = execute_cd(cmd); 
    }
    else if (strcmp(cmd[0], "clear") == 0) {
        execute_clear(cmd); 
        last_status = 0;     
    }
    else if (strcmp(cmd[0], "history") == 0) {            
        last_status = execute_history();
    }
    else if (strcmp(cmd[0], "ftype") == 0) {
        last_status = execute_ftype(cmd);
    }
    else if (strcmp(cmd[0], "kill") == 0) {
        last_status = execute_kill(cmd);
    }
    else if (strcmp(cmd[0], "exit") == 0) { // Comparer avec "exit"
        int exit_val = (cmd[1] != NULL)  ? atoi(cmd[1]) : last_status; // Obtenir le code de sortie
        exit(exit_val);
    }
    else {
        last_status = execute_external_command(cmd);
    }
    return last_status;
}

int execute_all_commands(char **cmds,int status) {
    int last_status = status;
    char **commande = malloc(64*sizeof(char*));
    if(commande==NULL){
        print("fsh: Erreur d'allocation\n", STDERR_FILENO);
        return 1;
    }
    int x=0;
    int y=0;    
    int entre_crochet=0;

    // Exécuter les commandes
    // Boucle pour traiter les tokens

    while (cmds[x]!=NULL){
        if (strcmp(cmds[x],";")==0 && entre_crochet==0){
            commande[y]=NULL;
            if(commande[0]!=NULL && y>0){
                last_status = execute_commande(commande,last_status);
                y=0;
            }
            else{
                print("fsh: Erreur de syntaxe\n", STDOUT_FILENO);
                last_status=1;
                break;
            }
        }
        else if (strcmp(cmds[x],"&&")==0 && entre_crochet==0){
            commande[y]=NULL;
            if(commande[0]!=NULL && y>0){
                last_status = execute_commande(commande,last_status);
                if(last_status!=0){
                    break;
                }
                y=0;
            }
            else{
                print("fsh: Erreur de syntaxe\n", STDOUT_FILENO);
                last_status=1;
                break;
                
            }
            if(last_status!=0){
                break;
            }
        }
        else if (strcmp(cmds[x],"{")==0){
            entre_crochet+=1;
            commande[y]=cmds[x];
            y++;
        }
        else if(strcmp(cmds[x],"}")==0){
            entre_crochet-=1;
            commande[y]=cmds[x];
            y++;
            if(cmds[x+1]==NULL){
                commande[y]=NULL;
                last_status = execute_commande(commande,last_status);
                y=0;
            }
        }
        else{
            commande[y] = cmds[x];
            if(cmds[x+1]==NULL){
                commande[y+1]=NULL;
                last_status = execute_commande(commande,last_status);
                y=0;
            }
            else{
                y++;
            }
        }
        x++;
    }
    free(commande);
    return last_status;
}

//decouper la ligne en tokens
char **argument(char *line, int *num_tokens) {
    int bufsize = 64;
    int position = 0;
    char **tokens = malloc(bufsize * sizeof(char*));
    if (!tokens) {
        print("fsh: allocation error\n", STDERR_FILENO);
        free(tokens);
        exit(EXIT_FAILURE);
    }

    char *token = malloc(strlen(line) + 1);
    if (!token) {
        print("fsh: allocation error\n", STDERR_FILENO);
        free(tokens);
        free(token);
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
                if(!tokens[position-1]){
                    print("fsh: allocation error\n", STDERR_FILENO);
                    free(token);
                    free(tokens);
                    exit(EXIT_FAILURE);
                }
                tok_pos = 0;
                in_token = 0;

                // Réallouer si nécessaire
                if (position >= bufsize) {
                    bufsize += 64;
                    tokens = realloc(tokens, bufsize * sizeof(char*));
                    if (!tokens) {
                        print("fsh: allocation error\n", STDERR_FILENO);
                        free(token);
                        free(tokens);
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

void free_tokens(char **tokens) {
    if (tokens) {
        for (int i = 0; tokens[i] != NULL; i++) {
            free(tokens[i]);
        }
        free(tokens);
    }
}



// gestion du signal 
void handle_sigint() {
    write(STDOUT_FILENO, "\n", 1);
    rl_on_new_line();
    rl_replace_line("", 0);
    rl_redisplay();
}

int main() {
    char *ligne;
    int last_status = 0;
    char prompt[1024]; // Buffer pour le prompt
    rl_outstream = stderr;

    // gestionnaires de signaux
    signal(SIGINT, handle_sigint);
    signal(SIGTSTP, SIG_IGN);

    rl_attempted_completion_function = completion; // pour Tab

    // Boucle principale du shell
    while (1) {
        // Construire le prompt
        afficher_prompt(last_status, prompt, sizeof(prompt));

        // Lire la ligne de commande avec readline en utilisant le prompt construit
        ligne = readline(prompt);

        if (!ligne) { // EOF (Ctrl-D)
            print("\n", STDOUT_FILENO);
            break;
        }

        // Si la ligne n'est pas vide, l'ajouter à l'historique
        if (strlen(ligne) > 0) {
            add_history(ligne);

            // Copier la ligne pour le traitement (strtok modifie la chaîne)
            char *line_copy = strdup(ligne);
            if (!line_copy) {
                print("strdup: ", STDERR_FILENO);
                print(strerror(errno), STDERR_FILENO);
                print("\n", STDERR_FILENO);
                free(ligne);
                continue;
            }

            //decouper les tokens
            int num_tokens = 0;
            char **tokens = argument(line_copy, &num_tokens);
            if (tokens==NULL) {
                print("fsh: Erreur d'allocation\n", STDERR_FILENO);
                free(tokens);
                free(line_copy);
                free(ligne);
                exit(EXIT_FAILURE);
            }
            last_status=execute_all_commands(tokens,last_status);
            free_tokens(tokens);
            free(line_copy);
        }
    
        free(ligne);
    }
    return last_status;
}   
