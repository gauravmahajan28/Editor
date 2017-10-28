/*
    20172045 : Gaurav Pramod Mahajan	
 *  controlTerminal.c : file containing functions related to 'how to control cursor movement on terminal and get terminal size
 */

// corrsponding header file
#include "controlTerminal.h"

// function to move cursor to required location
void gotoLocation(int xLocation, int yLocation)
{
	printf("%c[%d;%df", 0x1B, yLocation, xLocation);
}

// function to get console size
// ioctl is input output control, device specific system call
void getConsoleSize(struct winsize *terminal)
{
	ioctl(STDOUT_FILENO, TIOCGWINSZ, terminal);
}

// function to clear screen
// using escape sequence to clear screen
void clearScreen()
{
        printf("\e[1;1H\e[2J");
}


