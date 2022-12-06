#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include <sys/types.h>
#include <sys/wait.h>


// Fan  Processes

int main (int argc, char *argv[]){
    // argc is the count of the number of parameters in argv
    if (argc< 2){
        printf("You did not provide the number of child processes to create");
        exit(0);
    }

    // Convert the command input to an integer and loop through it.

    for(int i=0;i<(atoi(argv[1]));i++){
        // Child Process Creation 
        if(fork() == 0){
            printf("Hello! My pid is %d. Parent PID is %d\n", getpid(), getppid());
            exit(0);
        }

        // If creating child process fails
        if(fork < 0){
            printf("\nError creating Child Process");
            exit(1);
        }       
    }
    
    // Wait for the child processes to finish 
    for(int i=0;i<(atoi(argv[1]));i++){
        wait(NULL);
    }
    return 0;
}
