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
    enum {READ, WRITE};
    int pipeFD[2];

    if (pipe(pipeFD) < 0) {
        perror("Error in creating pipe");
        return;
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
        execvp(args1[0], args1);
        exit(0);
    } else {
        perror("FORK FAILED");
    }
}

void trim(char *str) {
    size_t len = strlen(str); // Length of the input string
    size_t start, end; // Start and end positions of the trimmed string

    // Find the start position of the trimmed string
    start = strspn(str, " \t"); // Skip leading spaces and tabs

    // Find the end position of the trimmed string
    end = len - 1;
    while (end > start && (str[end] == ' ' || str[end] == '\t')) {
        end--; // Skip trailing spaces and tabs
    }

    // Move the trimmed string to the beginning of the input string
    if (end > start) {
        memmove(str, &str[start], end - start + 1);
    }

    // Null-terminate the trimmed string
    str[end - start + 1] = '\0';
}

void splitTo(char input[], char **command, FILE **toFile) {
    *command = strtok(input, ">");
    char *to = strtok(NULL, ">");
    trim(to);
    *toFile = fopen(to, "w");
}

void splitFrom(char input[], char **command, FILE **fromFile) {
    *command = strtok(input, "<");
    char *from = strtok(NULL, "<");
    trim(from);
    *fromFile = fopen(from, "r");
}

int main(void) {
    char *args[MAX_LINE/2 + 1]; /* command line arguments */
    char *args2[MAX_LINE/2 + 1];
    char *pArgs[MAX_LINE/2 + 1];
    char *pArgs2[MAX_LINE/2 + 1];
    FILE *toFile = NULL;
    FILE *fromFile = NULL;
    int num_p_args = 0;
    int num_p_args2 = 0;
    int pIndicator = -1;

    while (1) {
        printf("osh>");
        fflush(stdout);

        char input[1024];
        fgets(input, 1024, stdin); // get line
        input[strcspn(input, "\n")] = 0; // replace \n with eol

        char *command = NULL;

        int indicator = -1;
        for (int i = 0; input[i] != '\0'; i++) {
            if (input[i] == '>') {
                indicator = 0;
                pIndicator = 0;
                splitTo(input, &command, &toFile);
                break;
            } else if (input[i] == '<') {
                indicator = 1;
                pIndicator = 1;
                splitFrom(input, &command, &fromFile);
                break;
            }
        }

        int num_args = 0, num_args2 = 0;
        char *first = NULL, *second = NULL;

        if (command != NULL) {
            num_args = tokenize(args, command);
        } else {
            pIndicator = -1;

            splitPipe(&first, &second, input); // first get string before, and second get after |

            num_args = tokenize(args, first); // args get strings in command

            num_args2 = tokenize(args2, second);
        }

        if (num_args == 0) {
            continue;
        }

        if (strcmp(args[num_args - 1], "&") == 1 || (num_args2 > 0 && strcmp(args2[num_args2 - 1], "&") == 1)) {

        }

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
            for (int i = 0; i < num_p_args2; i++) {
                free(pArgs2[i]);
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

                for (int i =0; i < num_p_args2; i++) {
                    free(pArgs2[i]);
                }

                // copy args
                num_p_args = num_args;
                for (int i = 0; i < num_args; i++) {
                    pArgs[i] = malloc(strlen(args[i]) + 1);
                    strcpy(pArgs[i], args[i]);
                }
                pArgs[num_args] = NULL;

                // copy args2
                num_p_args2 = num_args2;
                for (int i = 0; i < num_args2; i++) {
                    pArgs2[i] = malloc(strlen(args2[i]) + 1);
                    strcpy(pArgs2[i], args2[i]);
                }
                pArgs2[num_args2] = NULL;
            }

        } else if (pid == 0) {
            if (indicator == 0) {
                if (toFile == NULL) {
                    perror("No such file or directory");
                }
                dup2(fileno(toFile), 1);
                execvp(args[0], args);
            }
            else if (indicator == 1) {
                if (fromFile == NULL) {
                    perror("No such file or directory");
                }
                dup2(fileno(fromFile), 0);
                execvp(args[0], args);
            }
            else if (second != NULL && num_args2 >= 1) {
                execPipe(args, args2);
            }
            else if (strcmp(args[0], "!!") == 0 && num_args == 1) {
                if (num_p_args == 0) {
                    printf("No commands in history\n");
                }
                else if (pIndicator == 0) {
                    dup2(fileno(toFile), 1);
                    execvp(args[0], args);
                }
                else if (pIndicator == 1) {
                    dup2(fileno(fromFile), 0);
                    execvp(args[0], args);
                }
                else {
                    if (execvp(pArgs[0], pArgs) == -1) {
                        perror("!! FAILED\n");
                    }
                }
            }
            else {
                if (execvp(args[0], args) == -1) {
                    perror("EXEC FAILED\n");
                }
            }

            exit(0);

        } else {
            perror("FORK FAILED");
            return -1;
        }
    }

    return 0;
}
