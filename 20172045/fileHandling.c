/*
 * 20172045 : Gaurav Pramod Mahajan	
 * fileHandling.c : functions containing about file handling, reading file, saving file, rendering file
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <termios.h>

#include "fileHandling.h"
#include "controlTerminal.h"

// global variables maining information required by all modules
FILE *fp;
struct winsize ws;
int displayRowStartPosition = 1;
int displayRowEndPosition = 1;
extern int newMode, oldMode;

/*
 * startRowNumber : start row number in cells data structure to be displayed on terminal
 * endRowNumber : end row number in cells data structure to be displayed on terminal
 * Function sets startRow and endRow appropriately to be rendered on screen
 */
void setDisplayRowPosition(int startRowNumber, int endRowNumber)
{
	displayRowStartPosition = startRowNumber;
	displayRowEndPosition = endRowNumber;
}

// function to return current display start row position in data structure
int getDisplayRowStartPosition()
{
	return displayRowStartPosition;
}

// function to return curren display end row position in data structure
int getDisplayRowEndPosition()
{
	return displayRowEndPosition;
}

// function to print file data to screen
void printFileToScreen(FileMetadata *fileMetadata, Cell *cells[], RowMetadata *rowMetadata)
{
	// clearing screen at start
	clearScreen();
	getConsoleSize(&ws);

	// last row for status
	displayRowEndPosition = displayRowStartPosition + ws.ws_row - 2;  
	
	int rowCounter, lengthCounter = 1, bufferCounter, cellColumnCounter = 1;
	for(rowCounter = displayRowStartPosition; rowCounter <= displayRowEndPosition; rowCounter++)
	{
		for(lengthCounter = 1; lengthCounter <= rowMetadata[rowCounter].length; lengthCounter++)
		{
			for(bufferCounter = 0; bufferCounter < cells[rowCounter][cellColumnCounter].bufferLength; bufferCounter++)
			{
				if((lengthCounter % ws.ws_col == 1) && (lengthCounter / ws.ws_col >= 1))
				{
					printf("\n");
					displayRowEndPosition--;
				}
				else
				{	
					printf("%c",cells[rowCounter][cellColumnCounter].buffer[bufferCounter]);
				}
				lengthCounter++;
			}
			if(lengthCounter % ws.ws_col ==  1 && lengthCounter / ws.ws_col >= 1)
			{
				printf("\n");
				displayRowEndPosition--;
			}
			else
			{
				printf("%c",cells[rowCounter][cellColumnCounter].data);
				cellColumnCounter++;
			}
		}
		cellColumnCounter = 1;
		printf("\n");
		if(rowCounter >= fileMetadata->totalNumberOfRows)
			break;

	}
	gotoLocation(1, ws.ws_col);
	printf("mode = %s", ( newMode == 0 ? "DEFAULT_MODE" : (newMode == 1 ? "INSERT MODE :" : "COMMAND_MODE :")));

}


// function to readFile and initialize metadatas
/*
 * RowMetadata : containing length of each row
 * FileMetadata : contains total number of lines in file, fileName, whether file modified or not
 * Cells : actual data structure containing buffer in each cell to handle local inserts and deletes\
 * FileName : name of file to be read
 * CellLocation : actual location in data structure for cursorlocation in screen
 */


RowMetadata *initializeMetadatas(RowMetadata *rowMetadata, FileMetadata *fileMetadata, Cell **cells, char *fileName, CellLocation *cellLocation)
{
	fp = NULL;
	int fileExists = 1;

	// try to open file in readMode
	fp = fopen(fileName, "r");


	// try to open file in write mode
	if(fp == NULL)
	{
		fp = fopen(fileName, "w");
		fileExists = 0;
	}
	char ch;
	
	// maintaining indexing from 1,1 to match with screen console
	int rowCounter = 1;
	int colCounter = 1;

	//checking if file is empty or not
	int isFileEmpty = 0;
	long long numberOfBytes;
	if(fileExists == 1)
	{
		isFileEmpty = 0;
		fseek (fp, 0, SEEK_END);
		numberOfBytes = ftell(fp);
    		isFileEmpty = (ftell(fp) == 0) ? 1 : 0;
		fseek(fp, 0, SEEK_SET);
	}	

	
	if(fileExists == 0 || isFileEmpty)
		rowMetadata = (RowMetadata *)malloc(sizeof(RowMetadata) * 1024);
	else if(numberOfBytes  < 1024)
		rowMetadata = (RowMetadata *)malloc(sizeof(RowMetadata) * 1024);  // allocating enough rows .. can be decided on some better logic
	else
		rowMetadata = (RowMetadata *)malloc(sizeof(RowMetadata) * numberOfBytes); // assuming each byte occures in separate line
	
	//TODO : ADD LOGIC TO RESIZE NUMBER oF ROWS in CELLS


	// if file does not exist or file is empty
	// set metadatas
	if(fileExists == 0 || isFileEmpty)
	{
		clearScreen();
		// set length of each row as 0
		for(rowCounter = 0; rowCounter < 1024; rowCounter++)
		{
			rowMetadata[rowCounter].length = 0;
		}
		// set total number of rows
		(*fileMetadata).totalNumberOfRows = 0;
		// set file modified as 0
		(*fileMetadata).isFileModified = 0;
		// set file name
		strcpy((*fileMetadata).fileName, fileName);
		cellLocation->rowLocation = 1;
		cellLocation->colLocation = 1;
		return rowMetadata;
	}


	// read file character by character
	while(1)
	{
		ch = fgetc(fp);
		if(ch == '\n')
		{
			colCounter--;
			rowMetadata[rowCounter].length = colCounter;
			colCounter++;
			cells[rowCounter][colCounter].data = '\0';
			cells[rowCounter][colCounter].buffer = NULL;
			cells[rowCounter][colCounter].prefixLength = colCounter;
			cells[rowCounter][colCounter].bufferLength = 0;
			colCounter = 1;
			rowCounter++;
		}
		else if(ch == EOF)
		{
			rowCounter--;
			break;
		}
		else if(colCounter <= 1023)
		{
			cells[rowCounter][colCounter].data = ch;
			cells[rowCounter][colCounter].buffer = NULL;
			cells[rowCounter][colCounter].prefixLength = colCounter;
			cells[rowCounter][colCounter].bufferLength = 0;
			colCounter++;
		}
		else
		{
			// TODO : Allocate buffer on last cell to accomodate all incoming data
			if(cells[rowCounter][colCounter].bufferLength == 0)
			{
				cells[rowCounter][colCounter].buffer = (char *)malloc(sizeof(char) * 128); // assuming 128 more characters
			}
			cells[rowCounter][colCounter].buffer[(cells[rowCounter][colCounter].bufferLength)] = ch;
			(cells[rowCounter][colCounter].bufferLength)++;
			cells[rowCounter][colCounter].buffer[(cells[rowCounter][colCounter].bufferLength)] = '\0';
			(cells[rowCounter][colCounter].prefixLength)++;
		}	
	}
	// file has been read successfully

	// setting file metadata 
	(*fileMetadata).totalNumberOfRows = rowCounter;
	(*fileMetadata).isFileModified = 0;
	strcpy((*fileMetadata).fileName, fileName);
	
	// printing file to screen
	printFileToScreen(fileMetadata, cells, rowMetadata);
	// setting cell location
	cellLocation->rowLocation = 1;
	cellLocation->colLocation = 1;
	return rowMetadata;	

}

void saveToFile(FileMetadata *fileMetadata ,Cell *cells[], RowMetadata *rowMetadata)
{
	// close file
	fclose(fp);
	// open in write mode to overwrite content
	fp = fopen(fileMetadata->fileName, "w");

	if(fp == NULL)
	{
		printf("file save failed !! disk error..press enter to exit");
		getchar();
		return;
	}
	int rowCounter, lengthCounter, bufferCounter, cellColumnCounter = 1;
	// traversing cells data structure
	for( rowCounter = 1; rowCounter <= fileMetadata->totalNumberOfRows; rowCounter++)
	{
		for(lengthCounter = 1; lengthCounter <= rowMetadata[rowCounter].length; lengthCounter++)
		{
			for(bufferCounter = 0; bufferCounter < cells[rowCounter][cellColumnCounter].bufferLength; bufferCounter++)
			{
				fprintf(fp, "%c", cells[rowCounter][cellColumnCounter].buffer[bufferCounter]);
				lengthCounter++;
			}
			fprintf(fp, "%c", cells[rowCounter][cellColumnCounter].data);
			cellColumnCounter++;
		}	
		cellColumnCounter = 1;
		fprintf(fp, "\n");
	}
	fclose(fp);
}		
