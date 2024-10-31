#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <linux/limits.h>

int is_executable(const char *path) {
    struct stat sb;
    if (stat(path, &sb) == 0 && sb.st_mode & S_IXUSR) {
        return 1;
    }
    return 0;
}

// commandes internes
void list_internal_commands() {
    const char *internal_commands[] = {
        "ls", "pwd", "cd", "clear", "man", "tree", "open", "history", "exit", "compgen", NULL
    };

    for (int i = 0; internal_commands[i] != NULL; i++) {
        printf("%s\n", internal_commands[i]);
    }
}


int execute_compgen(int argc, char **argv) {
    // les arguments
    if (argc < 2) {
        fprintf(stderr, "Usage: compgen -c\n");
        return 1;
    }
    if (strcmp(argv[1], "-c") == 0) {
        list_internal_commands();
    } else {
        fprintf(stderr, "compgen: option inconnue: %s\n", argv[1]);
        return 1;
    }

    return 0;
}
