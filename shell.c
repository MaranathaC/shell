#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <stdlib.h>

#define MAX_LINE 80 /* The maximum length command */

void splitPipe(char **first, char **second, char input[]) {
    *first = strtok(input, "|"); // get before |
    if (*first != NULL) {
        *second = strtok(NULL, ""); // get after |
    }
}

int tokenize(char *args[], char *command) {
    int num_args = 0;
    args[num_args] = strtok(command, " ");
    while (num_args < 39 && args[num_args] != NULL) {
        args[++num_args] = strtok(NULL, " ");
    }

    args[MAX_LINE/2] = NULL;
    return num_args;
}

void execPipe(char *args1[], char *args2[]) {

}

int main(void) {
    char *args[MAX_LINE/2 + 1]; /* command line arguments */
    char *pArgs[MAX_LINE/2 + 1];
    char *pSecond[MAX_LINE/2 + 1];
    int num_p_args = 0;
    int num_p_sec;

    while (1) {
        printf("osh>");
        fflush(stdout);

        char input[1024];
        fgets(input, 1024, stdin); // get line
        input[strcspn(input, "\n")] = 0; // replace \n with eol

        char *first, *second;
        splitPipe(&first, &second, input); // first get string before, and second get after |

        int num_args = tokenize(args, first); // args get strings in command
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
            wait(&status);
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

            if (second != NULL && strlen(second) >= 1) {
                char *args2[MAX_LINE/2 + 1];
                tokenize(args2, second);

                enum {READ, WRITE};
                int pipeFD[2];

                if (pipe(pipeFD) < 0) {
                    perror("Error in creating pipe");
                    continue;
                }

                pid_t pid2 = fork();

                if (pid2 > 0) {
                    wait(NULL);
                    close(pipeFD[WRITE]);
                    dup2(pipeFD[READ], 0);
                    execvp(args2[0], args2);

                } else if (pid2 == 0) {
                    close(pipeFD[READ]);
                    dup2(pipeFD[WRITE], 1);
                    execvp(args[0], args);

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