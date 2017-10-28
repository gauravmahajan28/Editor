/*
 * 20172045 : Gaurav Pramod Mahajan
 */

#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <termios.h>


// point to linear movement within buffer 
typedef struct CursorLocation
{
	int xLocation;
	int yLocation;
}CursorLocation;	

// point to actual console movement on screen
typedef struct TerminalCursorLocation
{
	int xLocation;
	int yLocation;
	int currentRowNumber;
}TerminalCursorLocation;

// function to move cousor to required location
void gotoLocation(int xLocation, int yLocation);

// function to get console size
void getConsoleSize();

// fucntion to clear screen
void clearScreen();
