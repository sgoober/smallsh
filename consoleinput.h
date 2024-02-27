/*********************************************************************
** Program Filename: consoleinput.h
** Author: Sean Gibson
** Date: 2/22/2024
** Description: Header file for consoleinput.c
*********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>

//command is the first word in the input
//head is the head of a linked list of arguments
//input file is the first word after the symbol "<"
//output file the same after the symbol ">"
//control operator is true if the last character is "&"
struct consoleLine {
	char* command;
	struct argument* head;
	char* inputfile;
	char* outputfile;
	bool controloperator;
};

//Singly linked list of arguments 
struct argument {
	char* argument;
	struct argument* next;
};


struct consoleLine* addUniques(struct consoleLine*, char*, char*, int, char*);

struct consoleLine* consoleinputline();

void freeArguments_array(char**);

void freeArguments(struct consoleLine*);

void freeMemory(struct consoleLine*);


