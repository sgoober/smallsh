/*********************************************************************
** Program Filename: consoleinput.c
** Author: Sean Gibson
** Date: 2/22/2024
** Description: Deals with all user inputs: setting the input files,
** output files, arguments, command, and if it its a foreground or
** background process. It also sets the input and output streams,
** as well as freeing memory when applicable.
*********************************************************************/

#include "consoleinput.h"

//Takes in userinput line and sets appropriate information to the struct variables
struct consoleLine* addUniques(struct consoleLine* line, char* choice, char* argument, int length, char* choiceOriginal) {
	while (argument != NULL) {
		if (strcmp(argument, "<") == 0) {
			argument = strdup(strtok_r(choice, " ", &choice));
			line->inputfile = argument;
		}
		if (strcmp(argument, ">") == 0) {
			argument = strdup(strtok_r(choice, " ", &choice));
			line->outputfile = argument;
		}
		argument = strtok_r(choice, " ", &choice);
	}
	if (choiceOriginal[length-1] == '&') {
		line->controloperator = true;
	}
	return line;

}

//Takes in userinput and returns struct for user input line
struct consoleLine* consoleinputline() {
	struct consoleLine* line = (struct consoleLine*)malloc(sizeof(struct consoleLine));
	line->command = NULL;
	line->controloperator = false;
	line->head = NULL;
	line->inputfile = NULL;
	line->outputfile = NULL;

	char* choice = (char*)malloc(2048 * sizeof(char));

	char* choice2 = choice;

	int length = 0;
	printf(":");
	fflush(stdout);
	getline(&choice, &length, stdin);

	//get rid of endline and replace it with null terminator
	choice[strcspn(choice, "\n")] = '\0';


	int choicelength = strlen(choice);
	if (strspn(choice, " ") == choicelength) {
		free(choice);
		return line;
	}

	char* originalChoice = strdup(choice);

	int choiceLength = strlen(choice);

	line->command = strdup(strtok_r(choice, " ", &choice));

	//make linked list of argument unless it is <, >, &, or NULL
	char* argument = strtok_r(choice, " ", &choice);

	if (argument != NULL && (strcmp(argument, "<") != 0) && (strcmp(argument, ">") != 0) && (strcmp(argument, "&") != 0)) {
		struct argument* current = (struct argument*)malloc(sizeof(struct argument));
		struct argument* head = current;
		struct argument* temp = current;
		int arguments = 0;

		while (argument != NULL && (strcmp(argument, "<") != 0) && (strcmp(argument, ">") != 0) && (strcmp(argument, "&") != 0)) {
			arguments++;
			current->argument = strdup(argument);
			current->next = (struct argument*)malloc(sizeof(struct argument));
			temp = current;
			current = current->next;
			argument = strtok_r(choice, " ", &choice);
		}
		line->head = head;
		temp->next = NULL;
		if (arguments >= 512) {
			freeArguments(line);
			line->head = NULL;
		}
		free(current);
	}

	line = addUniques(line, choice, argument, choiceLength, originalChoice);

	choice = choice2;
	free(choice);
	free(originalChoice);

	return line;
}

//Takes in user input line and current directory path and returns array of arguments
char** argumentstoarray(struct consoleLine* line, char* path) {

	struct argument* pathwayarg = (struct argument*)malloc(sizeof(struct argument));
	struct argument* temp = line->head;
	if (line->command == NULL) {
		free(pathwayarg);
		return NULL;
	}

	if (strcmp(line->command, "ls") == 0 && (strcmp(path, ".") != 0)) { //Shouldn't be possible in the ssh but whatever :)
		
		pathwayarg->next = line->head;
		pathwayarg->argument = path;
		line->head = pathwayarg;
	}

	if (line->head == NULL) {
		free(pathwayarg);
		return NULL;
	}

	struct argument* head = line->head;
	struct argument* current = head;

	int count = 0;
	
	while (current != NULL) {
		count++;
		current = current->next;
	}

	char** args = (char**)malloc((count + 2) * sizeof(char*));

	current = head;
	args[0] = line->command;
	for (int i = 1; i < count+1; i++) {
		args[i] = current->argument;
		current = current->next;
	}

	args[count+1] = NULL;
	free(pathwayarg);

	if (strcmp(line->command, "ls") == 0) { //First argument of ls command should be the current directory
		line->head = temp;
	}
	return args;
}

//Takes in user input line (mostly the input file) and sets the input stream to it
int setInput(struct consoleLine* line) {
	int temp = dup(STDOUT_FILENO);
	int temp2 = dup(STDIN_FILENO);
	int temp3 = dup(STDERR_FILENO);


	int inputfile = open(line->inputfile, O_RDONLY);
	if (inputfile < 0) {
		dup2(temp, STDOUT_FILENO);
		dup2(temp2, STDIN_FILENO);
		dup2(temp3, STDERR_FILENO);

		printf("input file error\n");
		fflush(stdout);
		return 1;
	}
	dup2(inputfile, STDIN_FILENO);
	close(inputfile);
	return 0;
}

//Takes in user input line (mostly the output file) and sets the output stream to it
int setOutput(struct consoleLine* line) {
	int temp = dup(STDOUT_FILENO);
	int temp2 = dup(STDIN_FILENO);
	int temp3 = dup(STDERR_FILENO);
	
	int outputfile = open(line->outputfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (outputfile < 0) {
		dup2(temp, STDOUT_FILENO);
		dup2(temp2, STDIN_FILENO);
		dup2(temp3, STDERR_FILENO);
		printf("output file error\n");
		fflush(stdout);
		return 1;
	}
	dup2(outputfile, STDOUT_FILENO);
	dup2(outputfile, STDERR_FILENO);
	close(outputfile);
	return 0;
}

//Frees array of arguments (how it is passed for executing)
void freeArguments_array(char** args) {
	if (args == NULL) {
		return;
	}
	//for (int i = 0; args[i] != NULL; i++) {
	//	free(args[i]);
	//}
	free(args);
}

//Frees linked list of arguments (how its stored in struct)
void freeArguments(struct consoleLine* line) {
	struct argument* current = line->head;
	struct argument* next;
	while (current->next != NULL) {
		next = current->next;
		free(current->argument);
		free(current);
		current = next;
	}
	free(current->argument);
	free(current);
}

//Frees the memory allocated for the user input line struct
void freeMemory(struct consoleLine* line) {
	struct argument* current = line->head;
	struct argument* next;
	if (current != NULL) {
		while (current->next != NULL) {
			next = current->next;
			free(current->argument);
			free(current);
			current = next;
		}
		free(current->argument);
		free(current);
	}
	
	free(line->command);
	free(line->inputfile);
	free(line->outputfile);
	free(line);
	
}
