#include <stdio.h>
#include <unistd.h>
#include <string.h>

#define MAX_LINE 80 /* The maximum length command */

int main(void)
{
    char *args[MAX_LINE/2 + 1]; /* command line arguments */
    int should_run = 1; /* flag to determine when to exit program */

    while (should_run) {
        printf("osh>");
        fflush(stdout);

        char input[MAX_LINE];

        fgets(input, sizeof(input), stdin);

        // Tokenize the input string
        int i = 0;
        args[i] = strtok(input, " ");
        while (args[i] != NULL && i < MAX_LINE/2 + 1) {
            i++;
            args[i] = strtok(NULL, " ");
        }

        // Print the tokenized arguments
        for (int j = 0; j < i; j++) {
            printf("args[%d]: %s\n", j, args[j]);
        }

        /**
        * After reading user input, the steps are:
        * (1) fork a child process using fork()
        * (2) the child process will invoke execvp()
        * (3) parent will invoke wait() unless command included &
        */

        int pid = fork();

        if (pid == 0) {
            printf("Parent\n");
        } else if (pid > 0) {
            printf("Child\n");
        } else {
            printf("Failed\n");
        }
    }

    return 0;
}