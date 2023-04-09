#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <stdlib.h>

#define MAX_LINE 80 /* The maximum length command */
#define BUF_SIZE 4096

int main(void)
{
    char *args[MAX_LINE/2 + 1]; /* command line arguments */
    char *pArgs[MAX_LINE/2 + 1];
    char *currArgs[MAX_LINE/2 + 1];
    int num_p_args = 0;

    while (1) {
        printf("osh>");
        fflush(stdout);

        char input[1024];
        fgets(input, 1024, stdin);
        input[strcspn(input, "\n")] = 0;

        char *first = strtok(input, "|");
        char *second;
        if (first != NULL) {
            second = strtok(NULL, "");
            printf("Sec: %s\n", second);
        }

        char *command = strtok(input, ">");
        char *outputFile;
        if (command != NULL) {
            outputFile = strtok(NULL, ""); // Get the remaining part of the string after ">"
            printf("Output: %s\n", outputFile);
        }

        int num_args = 0;
        args[num_args] = strtok(command, " ");
        while (num_args < 39 && args[num_args] != NULL) {
            args[++num_args] = strtok(NULL, " ");
        }
        if (num_args == 0) {
            continue;
        }
        args[40] = NULL;

        /**
        * After reading user input, the steps are:
        * (1) fork a child process using fork()
        * (2) the child process will invoke execvp()
        * (3) parent will invoke wait() unless command included &
        */

        if (strcmp(args[0], "exit") == 0 && num_args == 1) {
            for (int i = 0; i < num_p_args; i++) {
                free(pArgs[i]);
            }
            break;
        }

        pid_t pid = fork();
        int status;

        if (pid > 0) {
            int cmp = strcmp(args[0], "!!");

            if (cmp != 0 || num_args > 1) { // free when not "!!"
                for (int i = 0; i < num_p_args; i++) {
                    free(pArgs[i]);
                }
                num_p_args = num_args;
                for (int i = 0; i < num_args; i++) {
                    pArgs[i] = malloc(strlen(args[i]) + 1);
                    strcpy(pArgs[i], args[i]);
                }
                pArgs[num_args] = NULL;
            }

        } else if (pid == 0) {
            if (strlen(second) >= 1) {
                enum {READ, WRITE};
                int pipeFD[2];

                if (pipe(pipeFD) < 0) {
                    perror("Error in creating pipe");
                    continue;
                }
                pid_t pid2 = fork();

                if (pid2 > 0) {

                } else if (pid2 == 0) {

                } else {
                    printf("FORK FAILED");
                }
            }

            if (strcmp(args[0], "!!") == 0 && num_args == 1) {
                if (execvp(pArgs[0], pArgs) == -1) {
                    printf("!! FAILED\n");
                }
            }
            else if (execvp(args[0], args) == -1) {
                printf("EXEC FAILED\n");
            }

        } else {
            printf("FORK FAILED");
            return -1;
        }
    }

    return 0;
}