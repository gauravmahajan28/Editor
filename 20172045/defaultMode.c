/*
 *   20172045 : Gaurav Pramod Mahajan
 *   defaultMode.c : handles actions taken in default mode
 */

#include <stdio.h>
#include <sys/ioctl.h>

#include "modes.h"
#include "keys.h"

struct winsize ws;

/** 
 * Tracking whether various keys pressed or not 
 **/
int gPressed = 0;
int rPressed = 0;
int escapePressed = 0;
// function to handle default mode input and return updated mode
int handleDefaultModeInput(char ch, CursorLocation *cursorLocation, int oldMode, RowMetadata *rowMetadata, FileMetadata *fileMetadata, Cell **cells, CellLocation *cellLocation, TerminalCursorLocation *terminalCursorLocation)
{

	getConsoleSize(&ws);
	int newMode;
	char temp1, temp2;

	// replace character
	// TODO : HANDLE FOR buffer character

	if(rPressed == 1)
	{
		// traversing to actual cell location
		// TODO : implement fast binary search
		int counter;
		for(counter = 1; counter < rowMetadata[cellLocation->rowLocation].length; counter++)
		{
			if(cursorLocation->xLocation <= cells[cellLocation->rowLocation][counter].prefixLength)
				break;
		}
		cellLocation->colLocation = counter;

		// we want to replace data value of cell
		if(cursorLocation->xLocation == cells[cellLocation->rowLocation][cellLocation->colLocation].prefixLength)
		{
			cells[cellLocation->rowLocation][cellLocation->colLocation].data = ch;
			rPressed = 0;
		}
		//we have to change buffer value 
		else
		{
			int bufferShift;
			if(cellLocation->colLocation == 1)
			{
				bufferShift = cursorLocation->xLocation;
			}
			else
			{
				bufferShift = cursorLocation->xLocation - cells[cellLocation->rowLocation][cellLocation->colLocation-1].prefixLength;
			}
			cells[cellLocation->rowLocation][cellLocation->colLocation].buffer[bufferShift - 1] = ch;
		}
		gotoLocation(terminalCursorLocation->xLocation, terminalCursorLocation->yLocation);
		printf("%c",ch);

		// move cursor to right
		(terminalCursorLocation->xLocation)++;
		// handling cursor at right case
		if(terminalCursorLocation->xLocation % ws.ws_col == 1 && cursorLocation->xLocation / ws.ws_col >= 1)
		{
			terminalCursorLocation->xLocation = 1;
			(terminalCursorLocation->yLocation)++;
		}
		gotoLocation(terminalCursorLocation->xLocation, terminalCursorLocation->yLocation);

		// adjusting cell
		(cursorLocation->xLocation)++;
		for(counter = 1; counter < rowMetadata[cellLocation->rowLocation].length; counter++)
		{
			if(cursorLocation->xLocation <= cells[cellLocation->rowLocation][counter].prefixLength)
				break;
		}
		cellLocation->colLocation = counter;

		return oldMode;
	}

	switch(ch)
	{

		case COLON:
			newMode = COMMAND_MODE;
			return newMode;

		case INSERT:
			newMode = INSERT_MODE;
			return newMode;

		case REPLACE:
			rPressed = 1;
			return oldMode;

		case GOTO_END:
			// setting cell row location
			cellLocation->rowLocation = (*fileMetadata).totalNumberOfRows;
			cellLocation->colLocation = 1;
			cursorLocation->xLocation = 1;
			terminalCursorLocation->xLocation = 1;

			if((*fileMetadata).totalNumberOfRows > (ws.ws_row - 1))
			{
				terminalCursorLocation->yLocation = ws.ws_row - 1;
				int endRowNumber = (*fileMetadata).totalNumberOfRows;
				int startRowNumber = endRowNumber - (ws.ws_row);
				setDisplayRowPosition(startRowNumber, endRowNumber);
				printFileToScreen(fileMetadata, cells, rowMetadata);
			}
			else
				terminalCursorLocation->yLocation = (*fileMetadata).totalNumberOfRows;
			gotoLocation(terminalCursorLocation->xLocation, terminalCursorLocation->yLocation);
			return oldMode;

		case GOTO_START:
			if(gPressed == 1)
			{
				cellLocation->rowLocation = 1;
				cellLocation->colLocation = 1;
				cursorLocation->xLocation = 1;
				terminalCursorLocation->xLocation = 1;
				terminalCursorLocation->yLocation = 1;
				terminalCursorLocation->currentRowNumber = 1;
				if(getDisplayRowStartPosition() > 1)
				{
					setDisplayRowPosition(1, ws.ws_row - 1);
					printFileToScreen(fileMetadata, cells, rowMetadata);
				}	
				gotoLocation(terminalCursorLocation->xLocation, terminalCursorLocation->yLocation);
				gPressed = 0;
			}
			else
			{
				gPressed = 1;
			}
			return oldMode;

		case LEFT: // left   // will never cause scrolling  ..done
			// move length counter to left
			(cursorLocation->xLocation)--;

			//adjust cell position
			int counter;
			for(counter = cellLocation->colLocation ;counter >= 2; counter--)
			{
				if((cursorLocation->xLocation) <= cells[cellLocation->rowLocation][counter].prefixLength && cursorLocation->xLocation > cells[cellLocation->rowLocation][counter-1].prefixLength)
					break;
			}	
			(cellLocation->colLocation) = counter;

			// we have reached start of row ..can't go to left
			if((cursorLocation->xLocation) < 1)
			{
				cursorLocation->xLocation = 1;
				(cellLocation->colLocation) = 1;
				terminalCursorLocation->xLocation = 1;
			}
			else
			{
				(terminalCursorLocation->xLocation)--;
				if((terminalCursorLocation->xLocation) % ws.ws_col == 1 && (cursorLocation->xLocation / ws.ws_col) >= 1)
				{
					terminalCursorLocation->xLocation = ws.ws_col;
					(terminalCursorLocation->yLocation)--;
				}
			}
			gotoLocation(terminalCursorLocation->xLocation, terminalCursorLocation->yLocation);
			return oldMode;

		case DOWN: // down

			// we have to compulsory go to next row

			(cellLocation->rowLocation)++;

			// we have gone below end of file
			if(cellLocation->rowLocation > fileMetadata->totalNumberOfRows)
			{

				cellLocation->rowLocation = fileMetadata->totalNumberOfRows + 1;
				cellLocation->colLocation = 1;
				cursorLocation->xLocation = 1;
				terminalCursorLocation->xLocation = 1;
				terminalCursorLocation->yLocation = fileMetadata->totalNumberOfRows + 1;
				terminalCursorLocation->currentRowNumber = cellLocation->rowLocation;
				rowMetadata[cellLocation->rowLocation].length = 0;
				if(terminalCursorLocation->yLocation == ws.ws_row)
					(terminalCursorLocation->yLocation)--;
				gotoLocation(terminalCursorLocation->xLocation, terminalCursorLocation->yLocation);
				return oldMode;

			}

			// we have incremented cellLocation->rowLocation.
			// we have to locate cellLocation->colLocation for cursorLocation->xLocation
			for(counter = 1; counter < rowMetadata[cellLocation->rowLocation-1].length; counter++)
			{
				if(cursorLocation->xLocation <= cells[cellLocation->rowLocation-1][counter].prefixLength)
					break;
			}
			cellLocation->colLocation = counter;


			// now we have to move cursor to appropriate location
			(terminalCursorLocation->currentRowNumber)++;

			for(counter = cursorLocation->xLocation; counter <=  rowMetadata[(cellLocation->rowLocation)-1].length; counter++)
			{
				if((counter % ws.ws_col == 1) && ((counter / (ws.ws_col)) >= 1))
				{
					(terminalCursorLocation->yLocation)++;
				}		
			}
			(terminalCursorLocation->yLocation)++;	
			(terminalCursorLocation->xLocation) = 0;

			for(counter = 1; counter <=  rowMetadata[cellLocation->rowLocation].length; counter++)
			{
				(terminalCursorLocation->xLocation)++;	
				if((counter % ws.ws_col == 1) && ((counter / ws.ws_col) >= 1))
				{
					terminalCursorLocation->xLocation = 1;
					(terminalCursorLocation->yLocation)++;
				}	
				if(counter == cursorLocation->xLocation)
					break;
			}

			if(getDisplayRowEndPosition() < terminalCursorLocation->currentRowNumber)
			{
				int startRow = getDisplayRowStartPosition();
				int endRow = getDisplayRowEndPosition();
				setDisplayRowPosition(startRow + 1, endRow +1);
				printFileToScreen(fileMetadata, cells, rowMetadata);
				(terminalCursorLocation->yLocation)--;
			}

			if(terminalCursorLocation->yLocation == ws.ws_row)
				(terminalCursorLocation->yLocation)--;
			gotoLocation(terminalCursorLocation->xLocation, terminalCursorLocation->yLocation);


			// we have to go to cusroLocation->xLocation of next row

			return oldMode;

		case UP: // up
			(cellLocation->rowLocation)--;

			// where we are trying to go above row1
			if(cellLocation->rowLocation < 1)
			{
				cellLocation->rowLocation = 1;
				terminalCursorLocation->yLocation = 1;
				terminalCursorLocation->currentRowNumber = 1;
				for(counter = 1; counter < rowMetadata[cellLocation->rowLocation+1].length; counter++)
				{
					if(cursorLocation->xLocation <= cells[cellLocation->rowLocation+1][counter].prefixLength)
						break;
				}
				cellLocation->colLocation = counter;
				gotoLocation(terminalCursorLocation->xLocation, terminalCursorLocation->yLocation);
				return oldMode;

			}

			// we have incremented cellLocation->rowLocation.
			// we have to locate cellLocation->colLocation for cursorLocation->xLocation
			for(counter = 1; counter < rowMetadata[cellLocation->rowLocation+1].length; counter++)
			{
				if(cursorLocation->xLocation <= cells[cellLocation->rowLocation+1][counter].prefixLength)
					break;
			}
			cellLocation->colLocation = counter;


			// now we have to move cursor to appropriate location
			(terminalCursorLocation->currentRowNumber)--;
			for(counter = cursorLocation->xLocation; counter >=  0; counter--)
			{
				if((counter % ws.ws_col == 1) && ((counter / (ws.ws_col)) >= 1))
				{
					(terminalCursorLocation->yLocation)--;
				}		
			}
			(terminalCursorLocation->yLocation)--;	
			(terminalCursorLocation->xLocation) = ((rowMetadata[cellLocation->rowLocation].length) % ws.ws_col);

			for(counter = rowMetadata[cellLocation->rowLocation].length; counter >= 0; counter--)
			{
				if(counter == cursorLocation->xLocation)
					break;
				(terminalCursorLocation->xLocation)--;	
				if((counter % ws.ws_col == 1) && ((counter / ws.ws_col) >= 1))
				{
					terminalCursorLocation->xLocation = ws.ws_col;
					(terminalCursorLocation->yLocation)--;
				}	
			}

			if(getDisplayRowStartPosition() > terminalCursorLocation->currentRowNumber)
			{
				int startRow = getDisplayRowStartPosition();
				int endRow = getDisplayRowEndPosition();
				setDisplayRowPosition(startRow - 1, endRow - 1);
				printFileToScreen(fileMetadata, cells, rowMetadata);
				(terminalCursorLocation->yLocation)--;
				terminalCursorLocation->yLocation = 1;
			}

			gotoLocation(terminalCursorLocation->xLocation, terminalCursorLocation->yLocation);


			// we have to go to cusroLocation->xLocation of next row

			return oldMode;

		case RIGHT: // right   // will never case scrolling  done
			(cursorLocation->xLocation)++;
			(terminalCursorLocation->xLocation) = ((terminalCursorLocation->xLocation) + 1);
			if(terminalCursorLocation->xLocation % ws.ws_col == 1)
			{
				(terminalCursorLocation->xLocation) = 1;
				(terminalCursorLocation->yLocation)++;
			}

			// switching to appropriate cell
			for(counter = cellLocation->colLocation ;counter < rowMetadata[cellLocation->rowLocation].length; counter++)
			{
				if((cursorLocation->xLocation) <= cells[cellLocation->rowLocation][counter].prefixLength)
					break;
			}	
			(cellLocation->colLocation) = counter;

			if((cursorLocation->xLocation) > (rowMetadata[(cellLocation->rowLocation)].length))
			{
				cursorLocation->xLocation = rowMetadata[(cellLocation->rowLocation)].length + 1;
				cellLocation->colLocation = rowMetadata[(cellLocation->rowLocation)].length + 1;
				terminalCursorLocation->xLocation = (cursorLocation->xLocation) % ws.ws_col;
			}
			gotoLocation(terminalCursorLocation->xLocation, terminalCursorLocation->yLocation);
			return oldMode;

		case ESCAPE: // escape
			temp1 = getchar();

			if(temp1== ARROW_START)  // arrow key presses
			{
				temp2 = getchar();
				switch(temp2)
				{
					case ARROW_UP: // arrow up
						(cellLocation->rowLocation)--;

						if(cellLocation->rowLocation < 1)
						{
							cellLocation->rowLocation = 1;
							cursorLocation->yLocation = 1;

							terminalCursorLocation->yLocation = 1;
							terminalCursorLocation->currentRowNumber = 1;
							for(counter = 1; counter < rowMetadata[cellLocation->rowLocation+1].length; counter++)
							{
								if(cursorLocation->xLocation <= cells[cellLocation->rowLocation+1][counter].prefixLength)
									break;
							}
							cellLocation->colLocation = counter;
							gotoLocation(terminalCursorLocation->xLocation, terminalCursorLocation->yLocation);
							return oldMode;

						}

						// we have incremented cellLocation->rowLocation.
						// we have to locate cellLocation->colLocation for cursorLocation->xLocation
						for(counter = 1; counter < rowMetadata[cellLocation->rowLocation+1].length; counter++)
						{
							if(cursorLocation->xLocation <= cells[cellLocation->rowLocation+1][counter].prefixLength)
								break;
						}
						cellLocation->colLocation = counter;


						// now we have to move cursor to appropriate location
						(terminalCursorLocation->currentRowNumber)--;
						for(counter = cursorLocation->xLocation; counter >=  0; counter--)
						{
							if((counter % ws.ws_col == 1) && ((counter / (ws.ws_col)) >= 1))
							{
								(terminalCursorLocation->yLocation)--;
							}		
						}
						(terminalCursorLocation->yLocation)--;	
						(terminalCursorLocation->xLocation) = ((rowMetadata[cellLocation->rowLocation].length) % ws.ws_col);

						for(counter = rowMetadata[cellLocation->rowLocation].length; counter >= 0; counter--)
						{
							if(counter == cursorLocation->xLocation)
								break;
							(terminalCursorLocation->xLocation)--;	
							if((counter % ws.ws_col == 1) && ((counter / ws.ws_col) >= 1))
							{
								terminalCursorLocation->xLocation = ws.ws_col;
								(terminalCursorLocation->yLocation)--;
							}	
						}

						if(getDisplayRowStartPosition() > terminalCursorLocation->currentRowNumber)
						{
							int startRow = getDisplayRowStartPosition();
							int endRow = getDisplayRowEndPosition();
							setDisplayRowPosition(startRow - 1, endRow - 1);
							printFileToScreen(fileMetadata, cells, rowMetadata);
							(terminalCursorLocation->yLocation)--;
							terminalCursorLocation->yLocation = 1;
						}

						gotoLocation(terminalCursorLocation->xLocation, terminalCursorLocation->yLocation);


						// we have to go to cusroLocation->xLocation of next row

						return oldMode;
					case ARROW_DOWN: // arrow down
						// we have to compulsory go to next row
						(cellLocation->rowLocation)++;

						// we have gone below end of file
						if(cellLocation->rowLocation > fileMetadata->totalNumberOfRows)
						{
							cellLocation->rowLocation = fileMetadata->totalNumberOfRows + 1;
							cellLocation->colLocation = 1;
							cursorLocation->xLocation = 1;
							terminalCursorLocation->xLocation = 1;
							terminalCursorLocation->yLocation = fileMetadata->totalNumberOfRows + 1;
							terminalCursorLocation->currentRowNumber = cellLocation->rowLocation;
							rowMetadata[cellLocation->rowLocation].length = 0;
							if(terminalCursorLocation->yLocation == ws.ws_row)
								(terminalCursorLocation->yLocation)--;
							gotoLocation(terminalCursorLocation->xLocation, terminalCursorLocation->yLocation);
							return oldMode;

						}

						// we have incremented cellLocation->rowLocation.
						// we have to locate cellLocation->colLocation for cursorLocation->xLocation
						for(counter = 1; counter < rowMetadata[cellLocation->rowLocation-1].length; counter++)
						{
							if(cursorLocation->xLocation <= cells[cellLocation->rowLocation-1][counter].prefixLength)
								break;
						}
						cellLocation->colLocation = counter;


						// now we have to move cursor to appropriate location
						(terminalCursorLocation->currentRowNumber)++;

						for(counter = cursorLocation->xLocation; counter <=  rowMetadata[(cellLocation->rowLocation)-1].length; counter++)
						{
							if((counter % ws.ws_col == 1) && ((counter / (ws.ws_col)) >= 1))
							{
								(terminalCursorLocation->yLocation)++;
							}		
						}
						(terminalCursorLocation->yLocation)++;	
						(terminalCursorLocation->xLocation) = 0;

						for(counter = 1; counter <=  rowMetadata[cellLocation->rowLocation].length; counter++)
						{
							(terminalCursorLocation->xLocation)++;	
							if((counter % ws.ws_col == 1) && ((counter / ws.ws_col) >= 1))
							{
								terminalCursorLocation->xLocation = 1;
								(terminalCursorLocation->yLocation)++;
							}	
							if(counter == cursorLocation->xLocation)
								break;
						}

						if(getDisplayRowEndPosition() < terminalCursorLocation->currentRowNumber)
						{
							int startRow = getDisplayRowStartPosition();
							int endRow = getDisplayRowEndPosition();
							setDisplayRowPosition(startRow + 1, endRow +1);
							printFileToScreen(fileMetadata, cells, rowMetadata);
							(terminalCursorLocation->yLocation)--;
						}

						if(terminalCursorLocation->yLocation == ws.ws_row)
							(terminalCursorLocation->yLocation)--;
						gotoLocation(terminalCursorLocation->xLocation, terminalCursorLocation->yLocation);


						// we have to go to cusroLocation->xLocation of next row

						return oldMode;

					case ARROW_RIGHT: // arrow right
						(cursorLocation->xLocation)++;
						(terminalCursorLocation->xLocation) = ((terminalCursorLocation->xLocation) + 1);
						if(terminalCursorLocation->xLocation % ws.ws_col == 1)
						{
							(terminalCursorLocation->xLocation) = 1;
							(terminalCursorLocation->yLocation)++;
						}

						for(counter = cellLocation->colLocation ;counter < rowMetadata[cellLocation->rowLocation].length; counter++)
						{
							if((cursorLocation->xLocation) <= cells[cellLocation->rowLocation][counter].prefixLength)
								break;
						}	

						(cellLocation->colLocation) = counter;
						if((cursorLocation->xLocation) > (rowMetadata[(cellLocation->rowLocation)].length))
						{
							cursorLocation->xLocation = rowMetadata[(cellLocation->rowLocation)].length + 1;
							cellLocation->colLocation = rowMetadata[(cellLocation->rowLocation)].length + 1;
							terminalCursorLocation->xLocation = (cursorLocation->xLocation) % ws.ws_col;
						}
						gotoLocation(terminalCursorLocation->xLocation, terminalCursorLocation->yLocation);

						return oldMode;

					case ARROW_LEFT: // arrow left
						(cursorLocation->xLocation)--;
						int counter;
						for(counter = cellLocation->colLocation ;counter >= 2; counter--)
						{
							if((cursorLocation->xLocation) <= cells[cellLocation->rowLocation][counter].prefixLength && cursorLocation->xLocation > cells[cellLocation->rowLocation][counter-1].prefixLength)
								break;
						}	

						(cellLocation->colLocation) = counter;

						if((cursorLocation->xLocation) < 1)
						{
							cursorLocation->xLocation = 1;
							(cellLocation->colLocation) = 1;
							terminalCursorLocation->xLocation = 1;

						}
						else
						{
							(terminalCursorLocation->xLocation)--;
							if((terminalCursorLocation->xLocation) % ws.ws_col == 1 && (cursorLocation->xLocation / ws.ws_col) >= 1)
							{
								terminalCursorLocation->xLocation = ws.ws_col;
								(terminalCursorLocation->yLocation)--;
							}
						}
						gotoLocation(terminalCursorLocation->xLocation, terminalCursorLocation->yLocation);
						return oldMode;

				}// inner switch
			} // inner if
			return oldMode;
	}
	return oldMode;
}
