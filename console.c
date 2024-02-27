/*********************************************************************
** Program Filename: console.c
** Author: Sean Gibson
** Date: 2/22/2024
** Description: Responsible for taking care of the array of background
** pids, breaking down the user inputs, making the array of arguments,
** updating the background processes and killing all the child processes.
*********************************************************************/

#include "console.h"

//Adds background pid to array of pids. Only resize if over 200 pids active
int* addpid(int* pids, int pid, int size) {
	if (size < 199) {
		pids[size] = pid;
		return pids;
	}
	else {
		pids = realloc(pids, (((size-199) * sizeof(int)) + 200 * sizeof(int)));
		pids[size] = pid;
	}

	return pids;
}

//Return different numbers based off the line
int understandline(struct consoleLine* line) {
	if (line->command == NULL) {
		return 1;
	}
	if (strcmp(line->command, "exit") == 0) {
		return 0;
	}
	if (line->command[0] == '#') {
		return 1;
	}
	if (strcmp(line->command, "cd") == 0) {
		return 2;
	}
	if (strcmp(line->command, "status") == 0) {
		return 3;
	}
	return 4;
}

//Sets all instances of "$$" in the argument arrays to the current pid
char** setDollarSigns_args(char** args, int pid) {
	for (int i = 0; args[i] != NULL; i++) {
		char* replacepid = strstr(args[i], "$$");
		while (replacepid != NULL) {
			//set $$ in args[i] to pid


			int sizeOfArgument = strlen(args[i]);
			char* newArg = malloc(sizeOfArgument + 10);
			strncpy(newArg, args[i], (replacepid - args[i]));
			sprintf(newArg + (replacepid - args[i]), "%d", pid);
			fflush(stdout);
			strcat(newArg, replacepid + 2);
			
			free(args[i]);
			args[i] = newArg;

			replacepid = strstr(args[i], "$$");
		}
	}
	return args;
}

//Sets the command, input, and output file names to pid number if there is a "$$"
struct consoleLine* setDollarSigns_line(struct consoleLine* line, int pid) {

	char* replacepid = strstr(line->command, "$$");
	if (replacepid != NULL) {
		int sizeOfCommand = strlen(line->command);
		char* tempnew = (char*)malloc(2048 * sizeof(char));
		strncpy(tempnew, line->command, (replacepid - line->command));
		sprintf(tempnew + (replacepid - line->command), "%d", pid);
		fflush(stdout);

		strcpy(tempnew + (replacepid - line->command) + 3, replacepid + 2);
		strcpy(line->command, tempnew);
		free(tempnew);

	}
	if (line->inputfile != NULL) {
		char* replacepid = strstr(line->inputfile, "$$");
		if (replacepid != NULL) {
			int sizeOfinput = strlen(line->inputfile);
			char* tempnew = (char*)malloc(2048 * sizeof(char));
			strncpy(tempnew, line->inputfile, (replacepid - line->inputfile));
			sprintf(tempnew + (replacepid - line->inputfile), "%d", pid);
			fflush(stdout);

			strcpy(tempnew + (replacepid - line->inputfile) + 3, replacepid + 2);
			strcpy(line->inputfile, tempnew);
			free(tempnew);
		}
	}
	if (line->outputfile != NULL) {
		char* replacepid = strstr(line->outputfile, "$$");
		if (replacepid != NULL) {
			int sizeOfinput = strlen(line->outputfile);
			char* tempnew = (char*)malloc(2048 * sizeof(char));
			strncpy(tempnew, line->outputfile, (replacepid - line->outputfile));
			sprintf(tempnew + (replacepid - line->outputfile), "%d", pid);
			fflush(stdout);

			strcpy(tempnew + (replacepid - line->outputfile) + 3, replacepid + 2);
			strcpy(line->outputfile, tempnew);
			free(tempnew);
		}
	}
	return line;
}

//Checks every pid in the pid array if they are complete (finished or terminated)
void backgroundProcesses(int* pids, int size) {

	for (int i = 0; i < size; i++) {
		if (pids[i] == NULL) {
			continue;
		}
		int pid = pids[i];
		int status;
		if (waitpid(pid, &status, WNOHANG) == 0) {
			continue;
		}

		if (WIFEXITED(status)) {
			printf("background pid %d is done: exit value %d\n", pid, WEXITSTATUS(status));
			pids[i] = NULL;
		}
		else if (WIFSIGNALED(status)) {
			printf("background pid %d is done: terminated by signal %d\n", pid, WTERMSIG(status));
			pids[i] = NULL;
		}
	}

	
}

//Kills every single kid in the pids array
void collateral(int* pids, int size) {
	for (int i = 0; i < size; ++i) {
		if (pids[i] != NULL) {
			kill(pids[i], SIGKILL);
			pids[i] = NULL;
		}
	}
}

