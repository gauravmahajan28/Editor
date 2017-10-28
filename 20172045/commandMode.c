/*
   20172045 : Gaurav Pramod Mahajan	
 * commandMode.c : it handles all the keyboard key strokes when editor is in command mode
 */	
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#include "modes.h"
#include "keys.h"
// handles command mode input and return updated mode if any

int handleCommandModeInput(char ch, CursorLocation *cursorLocation, int oldMode,  RowMetadata *rowMetadata, FileMetadata *fileMetadata, Cell *cells[], CellLocation *cellLocation, TerminalCursorLocation *terminalCursorLocation)
{
	int newMode;

	switch(ch)
	{
		case ESCAPE: // escape
			newMode = DEFAULT_MODE;

			getchar();

			return newMode;

		case SAVE_FILE:
			// saving file
			printf("w");
			getchar();
			saveToFile(fileMetadata, cells, rowMetadata);
			exit(0);
			newMode = DEFAULT_MODE;
			return newMode;
		

		case COMMAND_START:
			printf("!");
			char *command[128], temp;
			command[0] = (char *)malloc(sizeof(char) *32);
			command[1] = NULL;
			int rowCounter = 0;
			int counter = 0;
			while((temp = getchar()) != '\n')
			{
				if(temp == ' ')
				{
					command[rowCounter][counter] = '\0';
					command[++rowCounter] = (char *)malloc(sizeof(char) * 32);
					counter = 0;
					command[rowCounter+1] = NULL;
				}
				else
					command[rowCounter][counter++] = temp;
			}

			command[rowCounter][counter] = '\0';
	
			int pid = fork();

			if(pid == 0) // child
			{	
				clearScreen();
				gotoLocation(1, 1);
				execvp(command[0], command);
			}
			else // parent
			{
				wait(pid);
				printf("press enter to continue !");
				getchar();
				clearScreen();
				printFileToScreen(fileMetadata, cells, rowMetadata);
				gotoLocation(terminalCursorLocation->xLocation, terminalCursorLocation->yLocation);
				return DEFAULT_MODE;
			}

		case QUIT:
			// quiting
			printf("q");
			temp = getchar();
			if(temp == COMMAND_START)
			{
				printf("%c",temp);
				getchar();
				clearScreen();
				exit(0);
			}
			if(fileMetadata->isFileModified == 1)
			{
				struct winsize w;
				getConsoleSize(&w);
				gotoLocation(0, w.ws_col);
				printf("file has been modified !! Want to save file before quiting y / n");
				temp = getchar();
				if(temp == 'y')
				{
					saveToFile(fileMetadata, cells, rowMetadata);
					printf("file saved successfully..press enter to exit");
					getchar();
					exit(0);
				}
			}	
			newMode = DEFAULT_MODE;
			exit(0);
			return newMode;

	}
	return 0;
}
