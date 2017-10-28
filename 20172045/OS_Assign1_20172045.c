/* 20172045 : Gaurav Pramod Mahajan
 * Main file for editor
 */

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <termios.h>
#include <signal.h>
#include "controlTerminal.h"
#include "keyboardInput.h"
#include "fileHandling.h"

#define DEFAULT_MODE 0
#define INSERT_MODE 1
#define COMMAND_MODE 2

CursorLocation cursorLocation;
// initialize file metadata variables
RowMetadata *rowMetadata;
FileMetadata fileMetadata;
Cell **cells;
CellLocation cellLocation;
TerminalCursorLocation terminalCursorLocation;
int oldMode, newMode;

// print mode at end of screen
void printMode()
{
	struct winsize w;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
	gotoLocation(1, w.ws_col);
	printf("mode = %s", ( newMode == 0 ? "DEFAULT_MODE :" : (newMode == 1 ? "INSERT MODE :" : "COMMAND_MODE :")));
}

// handling screen change size event
void screenSizeChangeEvent(int signo)
{
	terminalCursorLocation.xLocation = 1;
	terminalCursorLocation.yLocation = 1;
	cellLocation.rowLocation = 1;
	cellLocation.colLocation = 1;
	cursorLocation.xLocation = 1;
	if(signo == SIGWINCH)
	{
		printFileToScreen(&fileMetadata, cells, rowMetadata);
	}
	printMode();
	gotoLocation(terminalCursorLocation.xLocation, terminalCursorLocation.yLocation);
}	


int main(int argc, char *argv[])
{
	char fileName[32];
	/**
	 *  Logic to handle file not specified while opening editor
	 **/
	if(argc != 2)
	{
		//TODO : change logic to generate new names every time
		// opening defualt newFile.txt  
		strcpy(fileName, "newfile.txt");
	}
	else
		strcpy(fileName,argv[1]);	
	

	// initializing memory
	cells = (Cell **)malloc(sizeof(Cell *) * 1024);
	int rowCounter;
	for( rowCounter = 0; rowCounter < 1024; rowCounter++)
		cells[rowCounter] = (Cell *)malloc(sizeof(Cell) * 1024);

	//initializing metadatas and reading file
	rowMetadata = initializeMetadatas(rowMetadata, &fileMetadata, cells, fileName, &cellLocation);

	/**
	 * Initializing cursor location in case file is present
	 **/
	cursorLocation.xLocation = 1;
	cursorLocation.yLocation = 1;
	terminalCursorLocation.xLocation = 1;
	terminalCursorLocation.yLocation = 1;
	terminalCursorLocation.currentRowNumber = 1;
	gotoLocation(terminalCursorLocation.xLocation, terminalCursorLocation.yLocation);

	/**
		MODE : 0 ----DEFAULT MODE
		MODE : 1 ----INSERT MODE
		MODE : 2 ----COMMAND MODE
	 **/
	oldMode = DEFAULT_MODE;
	newMode = DEFAULT_MODE;
	printMode();
	char ch;
	gotoLocation(terminalCursorLocation.xLocation, terminalCursorLocation.yLocation);

	/*
	 * Registering screen change size event
	 */
	signal(SIGWINCH, screenSizeChangeEvent);



	/**
	 *  Read input for ever until user exits application
	 **/
	while(1)
	{
		if(hasKeyboardHit()) // if keyboard has been hit
		{
			ch = getchar();

			if(oldMode == DEFAULT_MODE)
				newMode = handleDefaultModeInput(ch, &cursorLocation, oldMode, rowMetadata, &fileMetadata, cells, &cellLocation, &terminalCursorLocation);

			else if(oldMode == INSERT_MODE)
				newMode = handleInsertModeInput(ch, &cursorLocation, oldMode, rowMetadata, &fileMetadata, cells, &cellLocation, &terminalCursorLocation);

			else if(oldMode == COMMAND_MODE)
				newMode = handleCommandModeInput(ch, &cursorLocation, oldMode, rowMetadata, &fileMetadata, cells, &cellLocation, &terminalCursorLocation);


			if(oldMode == DEFAULT_MODE && newMode == INSERT_MODE)
			{
				oldMode = INSERT_MODE;
				printMode();
				gotoLocation(terminalCursorLocation.xLocation, terminalCursorLocation.yLocation);
				// move cursor to EOF location
			}

			if(oldMode == DEFAULT_MODE && newMode == COMMAND_MODE)
			{
				oldMode = COMMAND_MODE;
				printMode();
				// move cursor to EOF location
			}

			if(oldMode == INSERT_MODE && newMode == DEFAULT_MODE)
			{
				oldMode = DEFAULT_MODE;
				printMode();
				gotoLocation(terminalCursorLocation.xLocation, terminalCursorLocation.yLocation);
				// move cursor to EOF location
			}
			if(oldMode == COMMAND_MODE && newMode == DEFAULT_MODE)
			{
				oldMode = DEFAULT_MODE;
				printMode();
				gotoLocation(terminalCursorLocation.xLocation, terminalCursorLocation.yLocation);
				// move cursor to EOF location
			}

			clearBuffer();

		} // if keyboard hit
	}// while 1 for keep taking input
	return 0;
}
