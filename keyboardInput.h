
/*
 * 20172045 : Gaurav Pramod Mahajan
 */
#include <stdio.h>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <termios.h>


// function to clear input buffer
void clearBuffer();

// function to check whether keyboard has hit or not
int hasKeyboardHit();
