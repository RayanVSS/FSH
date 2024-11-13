#include <signal.h>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>

typedef struct {
    pid_t pid;
    char command[256];
} Job;


extern Job jobs[];
extern int job_count;

void fg_command(int job_id) {
    if (job_id > 0 && job_id <= job_count) {
        pid_t pid = jobs[job_id - 1].pid;
        if (kill(pid, SIGCONT) == -1) {
            perror("fg");
            return;
        }
        int status;
        if (waitpid(pid, &status, WUNTRACED) == -1) {
            perror("waitpid");
        }
        // Retirer le job de la liste si nécessaire
    } else {
        const char *msg = "fg: job inexistant\n";
        write(STDERR_FILENO, msg, strlen(msg));
    }
}
