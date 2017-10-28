
/*
 * 20172045 : Gaurav Pramod Mahajan
 */
// structure containing information about each row
typedef struct RowMetadata
{
	int length;
}RowMetadata;

// structure containing information about file
typedef struct FileMetadata
{
	int totalNumberOfRows;
	char fileName[32];
	int isFileModified;
}FileMetadata;


// structure containing each cell data and buffer to accomodate insert and delets locally
typedef struct Cell
{
	char data;
	char *buffer;
	int prefixLength;
	int bufferLength;
}Cell;


// structure pointing to actual location in data structure for cursor location
typedef struct CellLocation
{
	int rowLocation;
	int colLocation;
}CellLocation;	


// function to initialize all metadatas
RowMetadata *initializeMetadatas(RowMetadata *rowMetadata, FileMetadata *fileMetadata, Cell *cells[], char *fileName, CellLocation *cellLocation);


//function to print file data to screen at time of initialization
void printFileToScreen(FileMetadata *fileMetadata, Cell *cells[], RowMetadata *rowMetadata);

// getting start row number of current display
int getDisplayRowStartPosition();

// getting end row number of current display
int getDisplayRowEndPosition();

// setting appropriate start and end row number to handle scrolling
void setDisplayRowPosition(int startRowNumber, int endRowNumber);

// saving to file
void saveToFile(FileMetadata *fileMetadata ,Cell *cells[], RowMetadata *rowMetadata);

