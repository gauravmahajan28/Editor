/*
 * 201702045 : Gaurav Pramod Mahajan
 * insert mode.c :  handles input in insert mode
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/ioctl.h>

#include "keys.h"
#include "modes.h"
#include "keyboardInput.h"

struct winsize ws;
// using ~ for deleted

// function to handle insert mode input and returns updated mode
int handleInsertModeInput(char ch, CursorLocation *cursorLocation, int oldMode, RowMetadata *rowMetadata, FileMetadata *fileMetadata, Cell *cells[], CellLocation *cellLocation, TerminalCursorLocation *terminalCursorLocation)
{
	int newMode;
	// get console size
	getConsoleSize(&ws);
	// setting file modified as true
	int isCharPressed = 0;
	char temp1, temp2;

	isCharPressed = (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9') || (ch == 32) || (ch == '\n');

	if(isCharPressed == 1)
	{
		fileMetadata->isFileModified = 1;
		// inserting at end of line

		if( (cursorLocation->xLocation > rowMetadata[cellLocation->rowLocation].length && cursorLocation->xLocation < 1023) || (cursorLocation->xLocation == 1 && rowMetadata[cellLocation->rowLocation].length == 0))
		{
			// creating new row
			if(cellLocation->rowLocation > fileMetadata->totalNumberOfRows)
			{
				cells[cellLocation->rowLocation][cellLocation->colLocation].prefixLength = 0;
				rowMetadata[cellLocation->rowLocation].length = 0;
				(fileMetadata->totalNumberOfRows)++;
			}

			cells[cellLocation->rowLocation][cellLocation->colLocation].data = ch;
			cells[cellLocation->rowLocation][cellLocation->colLocation].buffer = NULL;
			cells[cellLocation->rowLocation][cellLocation->colLocation].prefixLength = cells[cellLocation->rowLocation][cellLocation->colLocation - 1].prefixLength;
			(rowMetadata[cellLocation->rowLocation].length)++;
			printf("%c",ch);
			(cursorLocation->xLocation)++;
			(cellLocation->colLocation)++;
			(terminalCursorLocation->xLocation)++;
			cells[cellLocation->rowLocation][cellLocation->colLocation].data = '\0';
			cells[cellLocation->rowLocation][cellLocation->colLocation].buffer = NULL;
			cells[cellLocation->rowLocation][cellLocation->colLocation].bufferLength = 0;
			cells[cellLocation->rowLocation][cellLocation->colLocation].prefixLength = cells[cellLocation->rowLocation][cellLocation->colLocation - 1].prefixLength;

			//oveflowing row
			if(((terminalCursorLocation->xLocation % ws.ws_col == 1) && (terminalCursorLocation->xLocation / ws.ws_col >= 1)) || ch == '\n')
			{
				terminalCursorLocation->xLocation = 1;
				(terminalCursorLocation->yLocation)++;
			}

			// if row flows then print entire file
			if(rowMetadata[cellLocation->rowLocation].length > ws.ws_col || ch == '\n')
			{	
				printFileToScreen(fileMetadata, cells, rowMetadata);
			}
			gotoLocation(terminalCursorLocation->xLocation, terminalCursorLocation->yLocation);



			// first empty file insert
			if(fileMetadata->totalNumberOfRows == 0)
				(fileMetadata->totalNumberOfRows)++;
			return oldMode;	

		}
		// handling local buffer inserts  : BUFFER points to data before current cell position
		else if(cursorLocation->xLocation <= rowMetadata[cellLocation->rowLocation].length)
		{
			int counter;
			for(counter = 1; counter < rowMetadata[cellLocation->rowLocation].length; counter++)
			{
				if(cursorLocation->xLocation <= cells[cellLocation->rowLocation][counter].prefixLength)
					break;
			}
			cellLocation->colLocation = counter;

			// using buffer to handle insert / deletes locally
			if(cells[cellLocation->rowLocation][cellLocation->colLocation].buffer == NULL)
			{
				cells[cellLocation->rowLocation][cellLocation->colLocation].buffer = (char *)malloc(sizeof(char) * 128);
			}

			// insert at end of end of buffer
			if(cursorLocation->xLocation == cells[cellLocation->rowLocation][cellLocation->colLocation].prefixLength)
			{

				int bufferLength = cells[cellLocation->rowLocation][cellLocation->colLocation].bufferLength;
				cells[cellLocation->rowLocation][cellLocation->colLocation].buffer[bufferLength] = ch;
				cells[cellLocation->rowLocation][cellLocation->colLocation].buffer[bufferLength+1] = '\0';
				(cells[cellLocation->rowLocation][cellLocation->colLocation].bufferLength)++;
				(cells[cellLocation->rowLocation][cellLocation->colLocation].prefixLength)++;

			}
			// insert in middle of buffer
			else
			{
				int locationToBeInserted;
				if(cellLocation->colLocation == 1)
					locationToBeInserted = cursorLocation->xLocation - 1;
				else
					locationToBeInserted = cursorLocation->xLocation - cells[cellLocation->rowLocation][cellLocation->colLocation - 1].prefixLength;
				int bufferLength = cells[cellLocation->rowLocation][cellLocation->colLocation].bufferLength;
				bufferLength++;
				int counter;
				for(counter = locationToBeInserted; counter < bufferLength; counter++)
				{
					char temp1 = cells[cellLocation->rowLocation][cellLocation->colLocation].buffer[locationToBeInserted];
					cells[cellLocation->rowLocation][cellLocation->colLocation].buffer[locationToBeInserted] = ch;
					ch = temp1;
					locationToBeInserted++;

				}
				cells[cellLocation->rowLocation][cellLocation->colLocation].buffer[counter] = '\0';
				(cells[cellLocation->rowLocation][cellLocation->colLocation].prefixLength)++;
				(cells[cellLocation->rowLocation][cellLocation->colLocation].bufferLength)++;

			}

			// updating prefix length of next cells
			int i;
			for(i = cellLocation->colLocation+1; i <= rowMetadata[cellLocation->rowLocation].length; i++)
			{
				if(cells[cellLocation->rowLocation][i].data == '\0')
					break;
				(cells[cellLocation->rowLocation][i].prefixLength)++;
			}	
			// updating length
			(rowMetadata[cellLocation->rowLocation].length)++;
			(terminalCursorLocation->xLocation)++;
			(cursorLocation->xLocation)++;
			if(((terminalCursorLocation->xLocation % ws.ws_col == 1) && (cursorLocation->xLocation / ws.ws_col >= 1)) || ch == '\n')
			{
				terminalCursorLocation->xLocation = 1;
				int counter;
				for(counter = 1; counter < rowMetadata[cellLocation->rowLocation].length; counter++)
				{
					if(cursorLocation->xLocation <= cells[cellLocation->rowLocation][counter].prefixLength)
						break;
				}
				cellLocation->colLocation = counter;
				(terminalCursorLocation->yLocation)++;
			}

			if( rowMetadata[cellLocation->rowLocation].length > ws.ws_col)
			{
				(cursorLocation->xLocation)++;
				printFileToScreen(fileMetadata, cells, rowMetadata);
				gotoLocation(terminalCursorLocation->xLocation, terminalCursorLocation->yLocation);
				return oldMode;
			}

		}

		return oldMode;
	}


	switch(ch)
	{
		// treating two escape hits as escape
		case ESCAPE:   // escape hit

			// if keyboard has not hit
			if(!hasKeyboardHit())
			{
				newMode = DEFAULT_MODE;
				return newMode;
			}	
			else
			{
				temp1 = getchar();
				if(temp1 == 91)    // DELETE ASCII SEQUENCE
				{
					temp2 = getchar();
					if(temp2 == 51) // DELETE ASCII SEQUENCE
					{
						fileMetadata->isFileModified = 1;

						if(cursorLocation->xLocation > rowMetadata[cellLocation->rowLocation].length)
							return oldMode;   // no character to delete
						else
						{
							int counter;
							for(counter = 1; counter < rowMetadata[cellLocation->rowLocation].length; counter++)
							{
								if(cursorLocation->xLocation <= cells[cellLocation->rowLocation][counter].prefixLength)
									break;
							}
							cellLocation->colLocation = counter;

							// if we are at data
							if(cursorLocation->xLocation == cells[cellLocation->rowLocation][cellLocation->colLocation].prefixLength)
							{

								cells[cellLocation->rowLocation][cellLocation->colLocation].data = '~';
								// copy until we get first buffer length non zero
								int colLocation = cellLocation->colLocation + 1;
								int foundBuffer = 0;
								while(cells[cellLocation->rowLocation][colLocation].data != '\0')
								{
									if(cells[cellLocation->rowLocation][colLocation].bufferLength != 0)
									{
										foundBuffer = 1;
										break;
									}
									else
									{
										cells[cellLocation->rowLocation][colLocation-1].data = cells[cellLocation->rowLocation][colLocation].data;
									}
									colLocation++;
								}
								if(foundBuffer)
								{
									cells[cellLocation->rowLocation][colLocation-1].data = cells[cellLocation->rowLocation][colLocation].buffer[0];
									(cells[cellLocation->rowLocation][colLocation].bufferLength)--;

									int counter;
									for(counter = 0; counter < cells[cellLocation->rowLocation][colLocation].bufferLength; counter++)
									{
										cells[cellLocation->rowLocation][colLocation].buffer[counter] = cells[cellLocation->rowLocation][colLocation].buffer[counter+1];
									}	
									cells[cellLocation->rowLocation][colLocation].buffer[counter]='\0';
								}
								else
								{
									cells[cellLocation->rowLocation][colLocation-1].data = '\0';
									cells[cellLocation->rowLocation][colLocation-1].prefixLength = 0;
									cells[cellLocation->rowLocation][colLocation-1].bufferLength = 0;
									cells[cellLocation->rowLocation][colLocation-1].buffer = NULL;
								}



								(rowMetadata[cellLocation->rowLocation].length)--;
								if(rowMetadata[cellLocation->rowLocation].length > ws.ws_col || 1)
								{
									printFileToScreen(fileMetadata, cells, rowMetadata);
								}
								gotoLocation(terminalCursorLocation->xLocation, terminalCursorLocation->yLocation);

							}
							// we are at buffer
							else
							{
								int locationToBeDeleted;
								if(cellLocation->colLocation == 1)
									locationToBeDeleted = cursorLocation->xLocation;
								else
									locationToBeDeleted = cursorLocation->xLocation - cells[cellLocation->rowLocation][cellLocation->colLocation - 1].prefixLength;

								int bufferLength = cells[cellLocation->rowLocation][cellLocation->colLocation].bufferLength;
								int counter;
								for(counter = locationToBeDeleted; counter < bufferLength; counter++)
								{
									cells[cellLocation->rowLocation][cellLocation->colLocation].buffer[counter] = cells[cellLocation->rowLocation][cellLocation->colLocation].buffer[counter+1];

								}
								cells[cellLocation->rowLocation][cellLocation->colLocation].buffer[locationToBeDeleted] = '\0';
								(cells[cellLocation->rowLocation][cellLocation->colLocation].bufferLength)--;
								(cells[cellLocation->rowLocation][cellLocation->colLocation].prefixLength)--;


								counter = cellLocation->colLocation + 1;
								while(cells[cellLocation->rowLocation][counter].data != '\0')
								{
									(cells[cellLocation->rowLocation][counter].prefixLength)--;
									counter++;
								}
								(rowMetadata[cellLocation->rowLocation].length)--;

								if(rowMetadata[cellLocation->rowLocation].length > ws.ws_col || 1)
								{
									printFileToScreen(fileMetadata, cells, rowMetadata);
								}
								gotoLocation(terminalCursorLocation->xLocation, terminalCursorLocation->yLocation);


							}
						}	
					}
				}

				return INSERT_MODE;
			}	

			break;


		case 127:   // backspace

			fileMetadata->isFileModified = 1;

			// TODO : delete only handled for deleting from last
			// first checking in cursor location is at end of row or not
			if(cursorLocation->xLocation > rowMetadata[cellLocation->rowLocation].length)
			{
				if(cursorLocation->xLocation == 1)
				{
					(cellLocation->rowLocation)--;
					// file is now empty
					if(cellLocation->rowLocation < 1)
					{
						cellLocation->rowLocation = 1;
						cellLocation->colLocation = 1;
						cursorLocation->xLocation = 1;
						terminalCursorLocation->xLocation = 1;
						terminalCursorLocation->yLocation = 1;
						gotoLocation(terminalCursorLocation->xLocation, terminalCursorLocation->yLocation);
						printFileToScreen(fileMetadata, cells, rowMetadata);
						return oldMode;
					}
					else
					{
						cursorLocation->xLocation = rowMetadata[cellLocation->rowLocation].length;
						int counter;
						for(counter = 1; counter < rowMetadata[cellLocation->rowLocation].length; counter++)
						{
							if(cursorLocation->xLocation <= cells[cellLocation->rowLocation][counter].prefixLength)
								break;
						}
						cellLocation->colLocation = counter;
						(terminalCursorLocation->yLocation)--;
						terminalCursorLocation->xLocation = (cursorLocation->xLocation % ws.ws_col)  + 1;
						gotoLocation(terminalCursorLocation->xLocation, terminalCursorLocation->yLocation);
						return oldMode;
					}

				}
				cells[cellLocation->rowLocation][cellLocation->colLocation - 1].data = '\0';
				cells[cellLocation->rowLocation][cellLocation->colLocation - 1].prefixLength = '\0';
				(rowMetadata[cellLocation->rowLocation].length)--;
				cursorLocation->xLocation--;
				if((terminalCursorLocation->xLocation) == 1 && cursorLocation->xLocation / ws.ws_col >= 1)
				{
					terminalCursorLocation->xLocation = ws.ws_col;
					(terminalCursorLocation->yLocation)--;
				}
				else
				{
					(terminalCursorLocation->xLocation)--;	
				}
				printFileToScreen(fileMetadata, cells, rowMetadata);
				gotoLocation(terminalCursorLocation->xLocation, terminalCursorLocation->yLocation);
				return oldMode;
			}
			else if(cursorLocation->xLocation >= 1 && cursorLocation->xLocation <= rowMetadata[cellLocation->rowLocation].length)
			{
				// position cell

				int counter;
				for(counter = 1; counter < rowMetadata[cellLocation->rowLocation].length; counter++)
				{
					if(cursorLocation->xLocation <= cells[cellLocation->rowLocation][counter].prefixLength)
						break;
				}
				cellLocation->colLocation = counter;

				// we are at first column
				if(cursorLocation->xLocation == 1)
					return oldMode;

				// we need to delete buffer
				else if(cursorLocation->xLocation <= cells[cellLocation->rowLocation][cellLocation->colLocation].prefixLength )	
				{
					int counter;
					int bufferLength = cells[cellLocation->rowLocation][cellLocation->colLocation].bufferLength;
					for(counter = cursorLocation->xLocation; counter <= bufferLength; counter++)
					{
						cells[cellLocation->rowLocation][cellLocation->colLocation].buffer[counter-1] = cells[cellLocation->rowLocation][cellLocation->colLocation].buffer[counter];

					}
					(cells[cellLocation->rowLocation][cellLocation->colLocation].bufferLength)--;
					(cells[cellLocation->rowLocation][cellLocation->rowLocation].prefixLength)--;
					((rowMetadata[cellLocation->rowLocation]).length)--;

					counter = cellLocation->colLocation + 1;
					while(cells[cellLocation->rowLocation][counter].data != '\0')
					{
						(cells[cellLocation->rowLocation][counter].prefixLength)--;
						counter++;
					}


				}	
				//  we need to delete previous data
				else if(cursorLocation->xLocation == cells[cellLocation->rowLocation][cellLocation->colLocation - 1].prefixLength + 1)
				{
					cells[cellLocation->rowLocation][cellLocation->colLocation-1].data = '~';
					// copy until we get first buffer length non zero
					int colLocation = cellLocation->colLocation;
					int foundBuffer = 0;
					while(cells[cellLocation->rowLocation][colLocation].data != '\0')
					{
						if(cells[cellLocation->rowLocation][colLocation].bufferLength != 0)
						{
							foundBuffer = 1;
							break;
						}
						else
						{
							cells[cellLocation->rowLocation][colLocation-1].data = cells[cellLocation->rowLocation][colLocation].data;
						}
						colLocation++;
					}
					if(foundBuffer)
					{
						cells[cellLocation->rowLocation][colLocation-1].data = cells[cellLocation->rowLocation][colLocation].buffer[0];
						(cells[cellLocation->rowLocation][colLocation].bufferLength)--;

						int counter;
						for(counter = 0; counter < cells[cellLocation->rowLocation][colLocation].bufferLength; counter++)
						{
							cells[cellLocation->rowLocation][colLocation].buffer[counter] = cells[cellLocation->rowLocation][colLocation].buffer[counter+1];
						}	
						cells[cellLocation->rowLocation][colLocation].buffer[counter]='\0';
					}
					else
					{
						cells[cellLocation->rowLocation][colLocation-1].data = '\0';
						cells[cellLocation->rowLocation][colLocation-1].prefixLength = 0;
						cells[cellLocation->rowLocation][colLocation-1].bufferLength = 0;
						cells[cellLocation->rowLocation][colLocation-1].buffer = NULL;
					}
					(rowMetadata[cellLocation->rowLocation].length)--;
				}

				(cursorLocation->xLocation)--;
				for(counter = 1; counter < rowMetadata[cellLocation->rowLocation].length; counter++)
				{
					if(cursorLocation->xLocation <= cells[cellLocation->rowLocation][counter].prefixLength)
						break;
				}
				cellLocation->colLocation = counter;

				if((terminalCursorLocation->xLocation % ws.ws_col == 1) && (cursorLocation->xLocation / ws.ws_col >= 1))
				{
					terminalCursorLocation->xLocation = ws.ws_col;
					(terminalCursorLocation->yLocation)--;
				}
				else
				{
					(terminalCursorLocation->xLocation)--;
				}
				printFileToScreen(fileMetadata, cells, rowMetadata);
				gotoLocation(terminalCursorLocation->xLocation, terminalCursorLocation->yLocation);
			}
			return oldMode;


		case 9: // horizontal tab  // handled only at end...if pressed in between, only cursor lovement changed

			if(cursorLocation->xLocation > rowMetadata[cellLocation->rowLocation].length)
			{
				// add 4 spaces
				int noOfSpacesToBeAdded = 4 - ((cursorLocation->xLocation) % 4);
				int counter;
				for(counter = 1; counter <= noOfSpacesToBeAdded; counter++)
				{
					cells[cellLocation->rowLocation][(cellLocation->colLocation)].data = ' ';
					cells[cellLocation->rowLocation][(cursorLocation->yLocation)].buffer = NULL;
					cells[cursorLocation->xLocation][(cursorLocation->yLocation)].bufferLength = 0;
					cells[cursorLocation->xLocation][(cursorLocation->yLocation)].prefixLength = cells[cursorLocation->xLocation][cursorLocation->yLocation-1].prefixLength + 1;
					printf(" ");
					(rowMetadata[cellLocation->rowLocation].length)++;
					(cursorLocation->xLocation)++;
					(terminalCursorLocation->xLocation)++;
					(cellLocation->colLocation)++;
					if((terminalCursorLocation->xLocation % ws.ws_col) == 1 && (cursorLocation->xLocation / ws.ws_col >= 1))
					{
						(terminalCursorLocation->xLocation) = 1;
						(terminalCursorLocation->yLocation)++;
						printFileToScreen(fileMetadata, cells, rowMetadata);
					}
					else if(cursorLocation->xLocation > ws.ws_col)
						printFileToScreen(fileMetadata, cells, rowMetadata);
					gotoLocation(terminalCursorLocation->xLocation, terminalCursorLocation->yLocation);
				}	
			}
			else
			{
				int noOfSpacesToBeAdded = 4 - ((cursorLocation->xLocation) % 4);
				cursorLocation->xLocation = cursorLocation->xLocation + noOfSpacesToBeAdded;
				if(cursorLocation->xLocation > rowMetadata[cellLocation->rowLocation].length)
				{	
					cursorLocation->xLocation = rowMetadata[cellLocation->rowLocation].length + 1;
					terminalCursorLocation->xLocation += cursorLocation->xLocation - rowMetadata[cellLocation->rowLocation].length;
				}
				else
				{
					int counter;
					for(counter = 1; counter <= noOfSpacesToBeAdded; counter++)
					{
						(terminalCursorLocation->xLocation)++;
						if(terminalCursorLocation->xLocation % ws.ws_col == 1 && cursorLocation->xLocation / ws.ws_col >= 1)
						{
							terminalCursorLocation->xLocation = 1;
							(terminalCursorLocation->yLocation)++;
						}	

					}
				}

				int counter;
				for(counter = 1; counter < rowMetadata[cellLocation->rowLocation].length; counter++)
				{
					if(cursorLocation->xLocation <= cells[cellLocation->rowLocation][counter].prefixLength)
						break;
				}
				cellLocation->colLocation = counter;
				gotoLocation(terminalCursorLocation->xLocation, terminalCursorLocation->yLocation);
			}
			return oldMode;

	}

	return INSERT_MODE; // return to default mode

}

