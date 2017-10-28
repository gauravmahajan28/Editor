/*
 * 20172045 : Gaurav Pramod Mahajan
 */


#define DEFAULT_MODE 0
#define INSERT_MODE 1
#define COMMAND_MODE 2

#define MODES 1

#include "controlTerminal.h"
#include "fileHandling.h"


// function to handle input depending on mode and return new mode if any
int handleDefaultModeInput(char ch, CursorLocation *cursorLocation, int oldMode, RowMetadata *rowMetadata, FileMetadata *fileMetadata, Cell *cells[], CellLocation *cellLocation, TerminalCursorLocation *terminalCursorLocation);

// function to handle input depending on mode and return new mode if any
int handleInsertModeInput(char ch, CursorLocation *cursorLocation, int oldMode, RowMetadata *rowMetadata, FileMetadata *fileMetadata, Cell *cells[], CellLocation *cellLocation, TerminalCursorLocation *terminalCursorLocation);

// function to handle input depending on mode and return new mode if any
int handleCommandModeInput(char ch, CursorLocation *cursorLocation, int oldMode, RowMetadata *rowMetadata, FileMetadata *fileMetadata, Cell *cells[], CellLocation *cellLocation, TerminalCursorLocation *terminalCursorLocation);
