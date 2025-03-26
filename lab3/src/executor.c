#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h> // Same as before

int global = 0;

int main(int argc, char *argv[]) {
	// If it's bigger than 2 we ignore the other arguments
	if (argc < 2) 
	{
		printf("Usage: %s <dirpath>\n", argv[0]);
		return 1;
	}
	
	int local = 0;
	
	printf("My name is: %s\n", argv[0]);
	
	pid_t pid = fork();
	
	if (pid < 0) 
	{
	    perror("fork failed");
	    return 1;
	}
       	else if (pid == 0) 
	{
		printf("Child process\n");
		global++;
		local++;
		printf("Child PID = %d, Parent PID = %d\n", getpid(), getppid());
		printf("Child's local = %d, Child's global = %d\n", local, global);	
		// execlp("ls","ls")
		execl("/bin/ls", "ls", argv[1], (char *)NULL);	
		perror("execl failed");
		exit(1);
	} 
	else 
	{
	    int status;
	    waitpid(pid, &status, 0);
	
	    printf("Parent process\n");
	    printf("Parent PID = %d, Child PID = %d\n", getpid(), pid);
	    printf("Child exit code: %d\n", WEXITSTATUS(status));
	    printf("Parent's local = %d, Parent's global = %d\n", local, global);
	}
	printf("Bonus original process parent id: %d, which sould be the process of currently runnig terminal:\n", getppid());
	perror("execl failed");
	printf("Here are the runnig bash terminals pids, there should be ours");
	execl("/bin/pidof", "pidof", "bash",(char *)NULL);
	perror("execl failed");
	exit(0);
	return 0;
}

