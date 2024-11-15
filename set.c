#include <unistd.h>
#include <string.h>

extern char **environ;

void set_command() {
    for (char **env = environ; *env != 0; env++) {
        write(STDOUT_FILENO, *env, strlen(*env));
        write(STDOUT_FILENO, "\n", 1);
    }
}
