#include  <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include  <sys/types.h>

int main(){
	char inputline[256]; //input from the user
	char *args[32];
	int i=0;
	int pid;
	while(i<5){
		printf("BasicShell: "); //print the shell prompt
		fgets(inputline, sizeof(inputline), stdin);	//get the use input
		inputline[sizeof(inputline)-1]='\0';
		
		/*need to place a \0 at the end of input 
		  for the call to exec()
		*/
		int j=0;
		while(inputline[j] !='\n'){
			j++;
		}
		inputline[j]='\0';
		
		//Now lets try to run the user input 
		*args=inputline;
		//we assume just one command!
		args[1]=NULL;
		
		//fork and use a child to run the command
		pid=fork();
		if(pid < 0){
			printf("\nThe fork failed\n");
			return(0);
		}
		else{
			if(pid == 0){
				//now in the child process, run the command
				printf("\n In the child and about to run %s \n", *args);
				execvp(*args, args);
				return(0);
			}
			else{
				//in the parent, wait for the child
				wait(NULL);
			}
		}
		i++;
	}
	return(0);
}
