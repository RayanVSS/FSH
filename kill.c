#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

void kill_command(pid_t pid, int signal) {
    if (kill(pid, signal) == -1) {
        perror("kill");
    }
}
