/*********************************************************************
** Program Filename: console.h
** Author: Sean Gibson
** Date: 2/22/2024
** Description: Header file for console.c
*********************************************************************/

#include "consoleinput.h"
#include <dirent.h>
#include <signal.h>


int* addpid(int*, int, int);

int understandline(struct consoleLine*);

char** setDollarSigns_args(char**, int);

struct consoleLine* setDollarSigns_line(struct consoleLine*, int);

void backgroundProcesses(int*, int);

void collateral(int*, int);