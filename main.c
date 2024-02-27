/*********************************************************************
** Program Filename: main.c
** Author: Sean Gibson
** Date: 2/22/2024
** Description: Runs getting user input in a loop and forking/executing
** appropriate commands.
*********************************************************************/

#include "console.h"

bool foregroundmode; //Global for if it is in foregroundmode

//Kills current foreground process if user inputs ctrl+c
void newSIGINT(int pid) {
	if (pid == 0) {
		return;
	}

	kill(pid, SIGKILL);
	int status;
	waitpid(pid, &status, WNOHANG);

	printf("terminated by signal %d\n", pid);
}

//Toggles foregroundmode if user inputs ctrl+z
void newSIGTSTP() {
	if (foregroundmode == false) {
		foregroundmode = true;
		printf("Entering foreground-only mode (& is now ignored)\n");
	}
	else {
		foregroundmode = false;
		printf("Exiting foreground-only mode (& is no longer ignored)\n");
	}
}

int main()
{
	//Variable declaration
	foregroundmode = false;
	char currentpath[260];
	char temppath[260];
	getcwd(currentpath, sizeof(currentpath));
	struct DIR* current_directory;
	struct dirent* entry;
	int ppid = 0;
	int pid = 0;
	int statuscode = 0;
	int status = 0;
	bool filefound = false;

	int* pids = (int*)malloc(200 * sizeof(int));
	int size = 0;
	pids[size] = NULL;

	//Signals to run the commands if user inputs SIGINT or SIGTSTP (ctrl+c or ctrl+z)
	signal(SIGINT, newSIGINT);
	signal(SIGTSTP, newSIGTSTP);

	//Saves the original input/output streams
	int oginput = dup(STDIN_FILENO);
	int ogoutput = dup(STDOUT_FILENO);


	//Single command line input is each loop
	while (true) {
		pid = 0;
		filefound = false;

		//resets input/output stream
		dup2(oginput, STDIN_FILENO);
		dup2(ogoutput, STDOUT_FILENO);

		//updates background processes
		backgroundProcesses(pids, size);

		//gets user input line and sets appropriate values
		struct consoleLine* line = consoleinputline();
		char** args = argumentstoarray(line, currentpath);

		if (line->inputfile != NULL) {
			if (setInput(line) == 1) { //failed to open
				statuscode = 1;
				freeMemory(line);
				freeArguments_array(args);
				continue;
			}
		}
		if (line->outputfile != NULL) {
			if (setOutput(line) == 1) { //failed to open
				statuscode = 1;
				freeMemory(line);
				freeArguments_array(args);
				continue;
			}
		}

		switch(understandline(line)) {
			case 0: //user inputs "exit"
				
				freeMemory(line);
				freeArguments_array(args);
				collateral(pids, size);
				
				free(pids);
				exit(0);
			case 1: //user inputs nothing or comment line
				break;
			case 2: //user inputs cd
				if (line->head == NULL) { //cd with no arguments takes you to homepath
					char* homePath = getenv("HOME");
					snprintf(currentpath, sizeof(currentpath), "%s", homePath);
				}
				else {
					//replace "$$" in cd to pid
					ppid = getpid();
					char* replacepid = strstr(line->head->argument, "$$");
					if (replacepid != NULL) {
						int sizeOfArgument = strlen(line->head->argument);
						char* newArg = malloc(sizeOfArgument + 10);
						strncpy(newArg, line->head->argument, (replacepid - line->head->argument));
						sprintf(newArg + (replacepid - line->head->argument), "%d", ppid);
						fflush(stdout);
						strcat(newArg, replacepid + 2);

						free(line->head->argument);
						line->head->argument = newArg;

					}

					//If command is cd -, remove most recent file from path
					if (strcmp(line->head->argument, "-") == 0) {
						char* lastSeparator = strrchr(currentpath, '/');
						if (lastSeparator != NULL) {
							*lastSeparator = '\0';
						} else {
							strcpy(currentpath, ".");
						}
						break;
					}
					//If argument starts with '/', treat as absolute pathway 
					if (line->head->argument[0] == '/') {
						DIR* absolute_directory = opendir(line->head->argument);
						if (absolute_directory == NULL) {
							printf("%s: No such file or directory\n", line->head->argument);
							fflush(stdout);
							closedir(absolute_directory);
							break;
						} else {
							if (entry->d_type == 0x4000) {
								filefound = true;
								snprintf(currentpath, sizeof(currentpath), "%s", line->head->argument);
								fflush(stdout);
							}
						}
						closedir(absolute_directory);
					}
					else { //Relative pathway
						current_directory = opendir(currentpath);
						while ((entry = readdir(current_directory)) != NULL) {
							if ((strcmp(entry->d_name, line->head->argument) == 0) && entry->d_type == DT_DIR) {
								filefound = true;
								if (strcmp(currentpath, ".") == 0) {
									snprintf(currentpath, sizeof(currentpath), "%s", entry->d_name);
								}
								else {
									strcpy(temppath, currentpath);
									snprintf(currentpath, sizeof(currentpath), "%s/%s", temppath, entry->d_name);
								}
							}
						}
						if (filefound == false) {
							printf("Directory: %s Not Found!\n", line->head->argument);
						}
						closedir(current_directory);
					}
				}
				break;
			case 3: //user inputs status
				printf("status code: %d\n", statuscode);
				fflush(stdout);
				break;
			case 4: //user inputs anything apart from cd, status, exit, comment, or blankline
				if (line->controloperator == false || foregroundmode == true) { //Foreground processes
					ppid = getpid();
					pid = fork();

					chdir(currentpath);

					if (pid == 0) {
						//child
						if (args != NULL) {
							args = setDollarSigns_args(args, ppid);
						}
						line = setDollarSigns_line(line, ppid);

						if (args == NULL) { //No args
							execlp(line->command, line->command, (char*)NULL);
							fflush(stdout); //flush output stream
							//Will only get to this if error is encountered executing
							printf("%s: No such file or directory!\n", line->command);
							exit(-1);
						}
						else { //Yes args
							execvp(line->command, args);
							fflush(stdout); //flush output stream
							//Will only get to this if error is encountered executing
							printf("%s: No such file or directory!\n", line->command);
							exit(-1);
						}
					}
					if (pid > 0) {
						//parent
						waitpid(pid, &status, 0);

						if (WEXITSTATUS(status) == 255) { //abnormal termination status code
							statuscode = -1;
						}
						else {
							statuscode = WEXITSTATUS(status);
						}
					}
				}
				else { //Background processes
					ppid = getpid();
					pid = fork();
					if (pid == 0) {
						//child
						if (args != NULL) {
							args = setDollarSigns_args(args, ppid);
						}
						line = setDollarSigns_line(line, ppid);
						if (line->inputfile == NULL) {
							int devNull = open("/dev/null", O_RDONLY);
							dup2(devNull, STDIN_FILENO);
						}
						if (line->outputfile == NULL) {
							int devNull = open("/dev/null", O_WRONLY);
							dup2(devNull, STDOUT_FILENO);
						}
						if (args == NULL) {
							execlp(line->command, line->command, (char*)NULL);
							fflush(stdout);
							printf("%s: No such file or directory!\n", line->command);
							exit(-1);
						}
						else {
							execvp(line->command, args);
							fflush(stdout);
							printf("%s: No such file or directory!\n", line->command);
							exit(-1);
						}
					}
					else {
						//parent
						printf("background pid is %d\n", pid);
						if (size == 0) {
							pids[0] = pid; //add first pid to array
							size += 1;
						}
						else {
							pids = addpid(pids, pid, size); //add pid to array and increase size for reallocation (if necessary)
							size += 1;
						}
					}
				}
				break;
		}
		//free memory of user input line and arguments before they go out of scope
		freeMemory(line);
		freeArguments_array(args);
	}
}