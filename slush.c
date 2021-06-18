#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <signal.h>

#define bufferSize 1024

void interrupt(int interruptSignal) {
    write(0, "\nKeyboard input interrupted\nslush$ ", 35);
}

int main(int argc, char* argv[]) {
    struct sigaction action;

    action.sa_flags = SA_RESTART;
    sigaction(SIGINT, &action, NULL);
    signal(SIGINT, interrupt);

    printf("Welcome to SLUShell");
    while (1)
    {
        printf("\nslush$ ");
        
        char buffer[bufferSize];

        char* retval = fgets(buffer,bufferSize, stdin);
        
        if ((strlen(buffer) > 0) && (buffer[-1] == '\n'))
        {
            buffer[-1] = NULL;
        }

        char* newLine = strchr(buffer, '\n');
        *newLine = '\0';

        char* cmd = "cd";
        char* commands[15];
        commands[0] = buffer;
        int equal = strncmp(buffer,cmd,2);
        if (equal == 0) {
            commands[0] = strtok(buffer," ");
            commands[1] = strtok(NULL, " ");
            int ret = chdir(commands[1]);
            if (ret == -1 ) {
                perror("could not change directory");
            }
            
        }

        char* newArgv[15];

        char *pointer = strtok(buffer, "(" );
        commands[0] = pointer;
        if (commands[0] == NULL) 
        {
            continue;
        }
        int i;
        for (i = 1; i <= 14; i++) 
        {
            pointer = strtok(NULL, "(" );
            commands[i] = pointer;
            if (pointer == NULL) 
            {
                commands[i] = '\0';
                break;
            }
        }

        int count;
        for (i = 0; i <= 14; i++) 
        {
            if ( commands[i] == NULL) 
            {
                count = i;
                break;
            }
        }
        int lastPipe;
        int fd[2];
        for (int i = count; i >= 0; i--) 
        {
            char *ptr = strtok(commands[i], " " );
            newArgv[0] = ptr;
            char *cmd = ptr;
            int j;
            for (j = 1; j <= 14; j++) 
            {
                ptr = strtok( NULL, " ");
                newArgv[j] = ptr;
                if (newArgv[j] == NULL) 
                {
                    newArgv[j] = '\0';
                    break;
                }
                
            }
            int innerCount = j;

            if (count == 0) {
                
                int child = fork();
                if (child == 0) {
                    int ret = execvp(cmd, newArgv);
                    if (ret == -1 ) {
                        printf("program %d: Not found\n", count+1);
                    }
                }
                else
                    wait(NULL);
            }
            else {
                if (i > 0)
                {
                    int retval = pipe(fd);
                    if (retval == -1)
                    {
                        perror("Could not create pipe");
                        exit(-1);
                    }      
                }
                int f1 = fork();
                if (f1 == 0) // child process
                {
                    if (i == count)
                    {
                        dup2(fd[1], STDOUT_FILENO);
                        close(fd[0]);
                    }
                    else if (i == 0)
                    {
                        dup2(lastPipe,STDIN_FILENO);
                    }
                    else
                    {
                        dup2(lastPipe, STDIN_FILENO);
                        dup2(fd[1], STDOUT_FILENO);
                        close(fd[0]);
                    }
                    int ret = execvp(newArgv[0],newArgv);
                    if (ret == -1 ) {
                        printf("program %d: Not found\n", count+1);
                    }
                }
                else
                {
                    if (i == 0)
                    {
                        close(lastPipe);
                    }
                    else
                    {
                        close(fd[1]);
                        if (i != count) 
                        {
                            close(lastPipe);
                        }
                        lastPipe = fd[0];
                    }
                }
            }
        }
        int k;
        for (k = 0; k <= count; k++)
        {
            wait(NULL);
        }
        
    }
}   