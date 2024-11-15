#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int execute_kill(pid_t pid, int signal) {
    if (kill(pid, signal) == -1) {
        perror("kill");
        return 1;
    }
    char buffer[120];
    snprintf(buffer, sizeof(buffer), "fsh: kill: le processus %ld a été tué\n", (long)pid);
    write(STDOUT_FILENO, buffer, strlen(buffer));
    return 0;
}

