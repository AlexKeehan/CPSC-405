#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/types.h>
#include <string.h>


int main(int argc, char *argv[])
{
	// To print our one error msg
	char error[30] = "An error has occurred\n";
	// This will tell if we are in batch mode or not
	int batch_mode = 1;
	// Just keeps the while loop going
	int true = 0;
	// Define our input buffer
	char input_buf[512];
	// pid for child processes
	int pid, status;
	// Will store our spliced string
	char *commands[32];
	// Flag for if a process should be run in background
	int bg = -1;
	// Flag for if there is a python file	
	int py = -1;
	// Keeps track of where the > is in redirection
	int redir_pos = -1;
	// Flag to keep track of redirection
	int redir = 0;
	
	// Checking to see if we are in batch mode
	if (argc == 2)
	{
		batch_mode = 0;
	}
	// Checking for too many command line arguments
	else if (argc > 2)
		batch_mode = 2;

	// Not in batch mode
	if (batch_mode == 1) {
		// Just keep looping until exit
		while (true == 0)
		{
			int x = 0;
			int y = 0;
				
			// Print our prompt
			printf("mysh> ");

			// Get user input
			fgets(input_buf, sizeof(input_buf), stdin);
			input_buf[sizeof(input_buf) - 1] = '\0';
				
			// Check if it overflows
			if (!strchr(input_buf, '\n')) {
				scanf("%*[^\n]");
				scanf("%*c");
				write(STDERR_FILENO, error, strlen(error));
				continue;
			}
			
			// Parse the input
			while (input_buf[x] != '\0')
			{
				while (input_buf[x] == ' ' || input_buf[x] == '\t' || input_buf[x] == '\n')
				{
					input_buf[x] = '\0';
					x++;
				}
				
				commands[y] = &input_buf[x];
				y++;		
				while (input_buf[x] != '\0' && input_buf[x] != ' ' && input_buf[x] != '\n')
				{
					x++;
				}
			}
			
			// Will store the size of commands/our spliced string
			int size = 0;

			// string stores pwd when called
			char string[32];
			// Check the size of commands
			for (int i=0;i<(y - 1);i++)
			{
				size++;
			}
			
			
			char * temp[32];
			int l = 0;
			// Check if the input string has no spaces like pwd>t.txt or ls>l.txt
			if (size == 1 && (strstr(commands[0], ">") || strstr(commands[0], "&"))) {
				// Check for no white spaces in redirection
				if (strstr(commands[0], ">")) {
					char *token = strtok(commands[0], ">");
						
					while (token != NULL) {
						temp[l++] = token;
						if ((token = strtok(NULL, ">")) != NULL) {
							temp[l++] = ">";
						}
					}
					size = l;
					// Copy temp back over to commands
					for (int x = 0; x < l; x++) {
						commands[x] = temp[x];
					}
				}
				// Check for no white spaces in background jobs
				else if (strstr(commands[0], "&")) {
					char *token = strtok(commands[0], "&");
						
					while (token != NULL) {
						temp[l++] = token;
						if ((token = strtok(NULL, "&")) != NULL) {
							temp[l++] = "&";
						}
					}
					size = l;
					// Copy temp back over to commands
					for (int x = 0; x < l; x++) {
						commands[x] = temp[x];
					}
				}
			}
			
			// If commands is empty then set the first element to null
			if (size == 0) {
				commands[y] = '\0';
			}
			// Check for redirection
			else {
				commands[size] = '\0';
				for (int i = 0; i < size; i++) {
					if (strcmp(commands[i], ">") == 0) {
						// Check for multiple >
						if (redir == 1) {
								write(STDERR_FILENO, error, strlen(error));
								continue;
							}
						redir_pos = i;
						redir = 1;
					}
				}
			}
			
			// Check for background jobs
			for (int i = 0; i < size; i++) {
				if (strcmp(commands[i], "&") == 0) {
					bg = 1;
				}
			}
			
			
			char *newcmd[32];
			newcmd[0] = "python3";
			// Check for python files
			if (size == 1) {
				if (strstr(commands[0], ".py") != NULL) {
					py = 1;
					int i = 0;
					while ((*(commands+i) != NULL) && i < 31) {
						newcmd[i+1] = *(commands+i);
						i++;
					}
					newcmd[i+1] = 0;
				}
			}

			int newfd;
			int stdout_copy = dup(1);
			// Begin checking for commands
			// Check if exit is called
			if (strcmp(commands[0], "exit") == 0)
			{
				exit(0);
			}
						
			// Check if cd is called
			else if (strcmp(commands[0], "cd") == 0)
			{	
				// Check if there is a second argument for cd
				// If so, use that as destination for chdir
				if (size == 2) 
				{
					chdir(commands[1]);
				}
				// If there is no argument, then go to home directory
				else if (size == 1)
				{
					char * home = getenv("HOME");
					chdir(home);	
				}
				// If there are more than 2 arguments to cd, then throw the error
				else
				{
					write(STDERR_FILENO, error, strlen(error));
				}
				
			}
			// Check for pwd
			else if (strcmp(commands[0], "pwd") == 0)
			{
				// Check for redirection within pwd
				if (redir == 1 && size >= 3 && size <= redir_pos + 2) {
					redir = 0;
					redir_pos = -1;
					newfd = open(commands[size - 1], O_CREAT|O_TRUNC|O_WRONLY, 0644);
					dup2(newfd, 1);
					// Print the current directory
					printf("%s \n", getcwd(string, sizeof(string)));
					dup2(stdout_copy, 1);
				}
				// No redirection
				else if (redir != 1) {
					printf("%s \n", getcwd(string, sizeof(string)));
				}
				else {
					write(STDERR_FILENO, error, strlen(error));
				}
			}
			// Check if there is nothing in commands
			else if (size == 0)
			{
				continue;
			}
			
			// Check for python files
			else if (py == 1) {
				py = 0;
				pid = fork();
				if (pid == 0) {
					int e = execvp(*newcmd, newcmd);
				}
				else
					while (wait(&status) != pid);
			}
			
			// Check for redirection
			else if (redir == 1) {
				redir = 0;
				// Check if command is in appropriate format
				if (size >= 3 && size <= redir_pos + 2) {
					newfd = open(commands[size - 1], O_CREAT|O_TRUNC|O_WRONLY, 0644);
					pid = fork();
					if (pid == 0)
					{
						dup2(newfd, 1);
						commands[redir_pos] = '\0';
						
						// Assign execvp to e to check for errors
						int e = execvp(*commands, commands);
						return(0);
					}
					// Wait in parent for child to finish
					else 
						while (wait(&status) != pid);
				}
				else
				{
					dup2(stdout_copy, 1);
					write(STDERR_FILENO, error, strlen(error));
				}
				dup2(stdout_copy, 1);
			}	
			
			// Check for built in commands
			else
			{
				// Fork off a child and run the command
				pid = fork();
				if (pid == 0)
				{
					if (bg == 1) {
						setpgid(0,0);
						commands[size - 1] = '\0';
					}
					
					// Assign execvp to e to check for errors
					int e = execvp(*commands, commands);
				// If there is an error, then print our error message
					if (e == -1) {
						write(STDERR_FILENO, error, strlen(error));
					}
				}
				// Wait in parent for child to finish
				else {
					if (bg == 1) {
						bg = -1;
						waitpid(pid, &status, WNOHANG);
					} else {
						while (wait(&status) != pid);
					}
				}
			}
		}
	}
	// In batch mode
	else if (batch_mode == 0) {
		char input_buf[512];
		char * commands[32];

		// Open the file
		FILE *file = fopen(argv[1], "r");
			
		// Check if the file is NULL
		if (file == NULL)
		{
			write(STDERR_FILENO, error, strlen(error));
			exit(0);
		}

		// Keep looping while there are lines in file
		while (fgets(input_buf, sizeof(input_buf), file) != NULL)
		{
			int x = 0;
			int y = 0;
			input_buf[sizeof(input_buf) - 1] = '\0';
			// Check if it overflows
			if (!strchr(input_buf, '\n')) {
				scanf("%*[^\n]");
				scanf("%*c");
				write(STDERR_FILENO, error, strlen(error));
				continue;
			}
		
			// Parse the input
			while (input_buf[x] != '\0')
			{
				while (input_buf[x] == ' ' || input_buf[x] == '\t' || input_buf[x] == '\n')
				{
					input_buf[x] = '\0';
					x++;
				}
			
				commands[y] = &input_buf[x];
				y++;		
				while (input_buf[x] != '\0' && input_buf[x] != ' ' && input_buf[x] != '\n')
				{
					x++;
				}
			}
			
			// Will store the size of commands/our spliced string
			int size = 0;

			// string stores pwd when called
			char string[32];
			// Check the size of commands
			for (int i=0;i<(y - 1);i++)
			{
				size++;
			}
			
			
			char * temp[32];
			int l = 0;
			// Check if the input string has no spaces like pwd>t.txt or ls>l.txt
			if (size == 1 && (strstr(commands[0], ">") || strstr(commands[0], "&"))) {
				// Check for no white spaces in redirection
				if (strstr(commands[0], ">")) {
					char *token = strtok(commands[0], ">");
						
					while (token != NULL) {
						temp[l++] = token;
						if ((token = strtok(NULL, ">")) != NULL) {
							temp[l++] = ">";
						}
					}
					size = l;
					// Copy temp back over to commands
					for (int x = 0; x < l; x++) {
						commands[x] = temp[x];
					}
				}
				// Check for no white spaces in background jobs
				else if (strstr(commands[0], "&")) {
					char *token = strtok(commands[0], "&");
						
					while (token != NULL) {
						temp[l++] = token;
						if ((token = strtok(NULL, "&")) != NULL) {
							temp[l++] = "&";
						}
					}
					size = l;
					// Copy temp back over to commands
					for (int x = 0; x < l; x++) {
						commands[x] = temp[x];
					}
				}
			}
			
			// If commands is empty then set the first element to null
			if (size == 0) {
				commands[y] = '\0';
			}
			// Check for redirection
			else {
				commands[size] = '\0';
				for (int i = 0; i < size; i++) {
					if (strcmp(commands[i], ">") == 0) {
						// Check for multiple >
						if (redir == 1) {
								write(STDERR_FILENO, error, strlen(error));
								continue;
							}
						redir_pos = i;
						redir = 1;
					}
				}
			}
			
			// Check for background jobs
			for (int i = 0; i < size; i++) {
				if (strcmp(commands[i], "&") == 0) {
					bg = 1;
				}
			}
			
			
			char *newcmd[32];
			newcmd[0] = "python3";
			// Check for python files
			if (size == 1) {
				if (strstr(commands[0], ".py") != NULL) {
					py = 1;
					int i = 0;
					while ((*(commands+i) != NULL) && i < 31) {
						newcmd[i+1] = *(commands+i);
						i++;
					}
					newcmd[i+1] = 0;
				}
			}

			int newfd;
			int stdout_copy = dup(1);
			// Begin checking for commands
			// Check if exit is called
			if (strcmp(commands[0], "exit") == 0)
			{
				exit(0);
			}
						
			// Check if cd is called
			else if (strcmp(commands[0], "cd") == 0)
			{	
				// Check if there is a second argument for cd
				// If so, use that as destination for chdir
				if (size == 2) 
				{
					chdir(commands[1]);
				}
				// If there is no argument, then go to home directory
				else if (size == 1)
				{
					char * home = getenv("HOME");
					chdir(home);	
				}
				// If there are more than 2 arguments to cd, then throw the error
				else
				{
					write(STDERR_FILENO, error, strlen(error));
				}
				
			}
			// Check for pwd
			else if (strcmp(commands[0], "pwd") == 0)
			{
				// Check for redirection within pwd
				if (redir == 1 && size >= 3 && size <= redir_pos + 2) {
					redir = 0;
					redir_pos = -1;
					newfd = open(commands[size - 1], O_CREAT|O_TRUNC|O_WRONLY, 0644);
					dup2(newfd, 1);
					// Print the current directory
					printf("%s \n", getcwd(string, sizeof(string)));
					dup2(stdout_copy, 1);
				}
				// No redirection
				else if (redir != 1) {
					printf("%s \n", getcwd(string, sizeof(string)));
				}
				else {
					write(STDERR_FILENO, error, strlen(error));
				}
			}
			// Check if there is nothing in commands
			else if (size == 0)
			{
				continue;
			}
			
			// Check for python files
			else if (py == 1) {
				py = 0;
				pid = fork();
				if (pid == 0) {
					int e = execvp(*newcmd, newcmd);
				}
				else
					while (wait(&status) != pid);
			}
			
			// Check for redirection
			else if (redir == 1) {
				redir = 0;
				// Check if command is in appropriate format
				if (size >= 3 && size <= redir_pos + 2) {
					newfd = open(commands[size - 1], O_CREAT|O_TRUNC|O_WRONLY, 0644);
					pid = fork();
					if (pid == 0)
					{
						dup2(newfd, 1);
						commands[redir_pos] = '\0';
						
						// Assign execvp to e to check for errors
						int e = execvp(*commands, commands);
						return(0);
					}
					// Wait in parent for child to finish
					else 
						while (wait(&status) != pid);
				}
				else
				{
					dup2(stdout_copy, 1);
					write(STDERR_FILENO, error, strlen(error));
				}
				dup2(stdout_copy, 1);
			}	
			
			// Check for built in commands
			else
			{
				// Fork off a child and run the command
				pid = fork();
				if (pid == 0)
				{
					if (bg == 1) {
						setpgid(0,0);
						commands[size - 1] = '\0';
					}
					
					// Assign execvp to e to check for errors
					int e = execvp(*commands, commands);
				// If there is an error, then print our error message
					if (e == -1) {
						write(STDERR_FILENO, error, strlen(error));
					}
				}
				// Wait in parent for child to finish
				else {
					if (bg == 1) {
						bg = -1;
						waitpid(pid, &status, WNOHANG);
					} else {
						while (wait(&status) != pid);
					}
				}
			}
		}
	}
}



