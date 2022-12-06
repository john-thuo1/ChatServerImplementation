
// C program to create a chain of process
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

// Chain of Processes

int main(int argc, char *argv[])
{
	int processid;

    // argc is the count of the number of parameters in argv

    if (argc< 2){
        printf("You did not provide the number of child processes to create");
        exit(1);
    }
	
    // Convert the command input to an integer and loop through it.

	for (int i = 0; i < (atoi(argv[1])); i++) {
		processid = fork();

		if (processid > 0) {
			wait(NULL);
            break;
		}
		else {
			// Print the child process with parent ID information
	        printf("Hello! My pid is %d. Parent PID is %d\n", getpid(), getppid());

            wait(NULL);
		}
	}

	return 0;
}
