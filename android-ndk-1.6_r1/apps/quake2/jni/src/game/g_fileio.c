// debut mod : file
#include "g_local.h"

FILE *in;				           // the file
char *filename;				        // the file's name		
qboolean FileLoaded = false;           	// whether or not the file is loaded (defaults to no file loaded)

void LoadFile(edict_t *ent)
{
	if (!FileLoaded) {	// if there is not already a file open
	
		filename = gi.args();	// copy the desired filename into 'filename' 
	
		if ((in = fopen(filename, "r")) == NULL) {	// test to see if file opened 
			// file did not load
			gi.cprintf (ent, PRINT_HIGH, "Could not load file %s.\n", filename);
			FileLoaded = false;
		}
		else {
			// file did load
			gi.cprintf(ent, PRINT_HIGH, "File %s loaded.\n", filename);
			FileLoaded = true;
		}
	}
	else	// there's already a file open
		gi.cprintf(ent, PRINT_HIGH, "File %s is already loaded.\n, filename");
}


void CloseFile(edict_t *ent)
{
	if (FileLoaded) {	// if there is a file open
		fclose(in);		// use STL to close it
		FileLoaded = false;
		gi.cprintf(ent, PRINT_HIGH, "File %s closed.\n", filename);	// notify user
	}
	else	// no file is opened
		gi.cprintf(ent, PRINT_HIGH, "No file is currently loaded.\n");
}


void ShowFile(edict_t *ent)
{
	int c;	// variable to hold temporary file data

	if (FileLoaded) {	// if there is a file loaded
		// this while loop completes two tasks: checks to see if we
		// are at the end of the file, and assigns 'c' to the data
		// at the position of the file pointer
		while ((c = fgetc(in)) != EOF)
			gi.cprintf(ent, PRINT_HIGH, "%c", c);	// output 'c'
		gi.cprintf(ent, PRINT_HIGH, "\nend of %s", filename);
	}
	else	// no file is loaded
		gi.cprintf(ent, PRINT_HIGH, "No file is currently loaded.\n");
}
// fin mod : file
