/*
 * 20172045 : Gaurav Pramod Mahajan
 * keyboardInput.c : file containing function to clear buffer and check whether keyboardHasHit
 */

#include "keyboardInput.h"

// function to clear input buffer
/*
 * Function keep of reading buffer untill it encounters EOF on stdin	
 */
void clearBuffer()
{
	while(feof(stdin))
		getchar();
}


// function to check whether keyboard has hit or not
// NOTE : FUNCTION HAS BEEN REFERRED FROM GOOGLE
/*
 * Function sets terminal in non canonical mode and reads one character
 */
int hasKeyboardHit()
{
	struct termios oldState, newState;
	int oldFunction;
	int character;

	// getting terminal attributes in old state
	tcgetattr(STDIN_FILENO, &oldState);
	newState = oldState;
	// setting terminal to non canonical mode and non echo mode
	newState.c_lflag &= ~(ICANON | ECHO);
	//setting new terminal settings
	tcsetattr(STDIN_FILENO, TCSANOW, &newState);

	// fcntl manipulates file descriptor
	// Getting file status flag for terminal
	oldFunction = fcntl(STDIN_FILENO, F_GETFL, 0);
	//adding non blocking mode i.e.  every function has to return immediately , non blocking 
	fcntl(STDIN_FILENO, F_SETFL, oldFunction | O_NONBLOCK);

	character = getchar();
	// setting old settings
	tcsetattr(STDIN_FILENO, TCSANOW, &oldState);
	// setting old file status flag for terminal
	fcntl(STDIN_FILENO, F_SETFL, oldFunction);

	// if some character is present then return 1 and add character to buffer
	if(character != EOF)
	{
		ungetc(character, stdin);  // putting ch back on to buffer
		return 1;
	}
	else 
		ungetc(character, stdin);  // putting ch back on to buffer ->to handle differentiate between escape and escape sequence

	return 0;

}
