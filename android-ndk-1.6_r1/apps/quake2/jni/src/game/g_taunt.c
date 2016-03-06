// debut mod : taunt

// g_taunt.c
#include "g_local.h" // talk to Quake

// macros
#define myrandom() ((int)(random()*100))	// this returns a random number between 0 and 100

// defines
#define VERB		0	// these #defines help GetTaunt()
#define ADJECTIVE	1	// know what type of word to
#define NOUN		2	// return

#define MAX_OBJECTS		750	// max storage space for verb, adjective, and noun arrays
#define MAX_LINE_SIZE	100 // maximum number of characters to read in per line of a file


// global variables
int TauntInit = 0;	// 0 if taunt did not load, 1 if it did

char Verbs[MAX_OBJECTS],		// these arrays contain all of our
	 Adjectives[MAX_OBJECTS],	// verbs, adjectives, and nouns
	 Nouns[MAX_OBJECTS];		// for use in taunts

int NumVerbs = 0,		// these hold the number of elements
	NumAdjectives = 0,  // in each array
	NumNouns = 0;


// declarations
void RandomTaunt(char s[], int num, char elements[]);
void GetTaunt(char s[], int type);
int GetLineFromFile(FILE *in, char s[]);
int SearchFile(FILE *in, char s[]);


// -- FUNCTIONS
//
// Loads taunts.txt, moves Verbs, Adjectives, and Nouns into their
// own arrays. This is called by InitGame().
void InitTaunt(void)
{
	FILE *in;	// taunts.txt
	char CurrentLine[MAX_LINE_SIZE];	// the line we are currently sorting

	if ((in = fopen("taunts.txt", "r")) == NULL) {	// taunts.txt could not be loaded
		// print an error message, set TauntInit to 0, and exit
		gi.dprintf("==== InitTaunt: Could not load file: taunts.txt ====\n");
		TauntInit = 0;
		return;
	}
	
	// Search our file for [VERBS]
	if (!SearchFile(in, "[VERBS]"))
		// if [VERBS] is not found, return, TauntInit will still be set at 0
		return;
	// After [VERBS] is found, read from the file a line at a time
	// until a line with nothing on it is reached
	while (GetLineFromFile(in, CurrentLine) > 1) {
		// concatenate the current line into our verbs array
		strcat(Verbs, CurrentLine);
		NumVerbs++;
	}

	// Do the same for both adjectives and nouns
	if (!SearchFile(in, "[ADJECTIVES]"))
		return;
	while (GetLineFromFile(in, CurrentLine) > 1) {
		strcat(Adjectives, CurrentLine);
		NumAdjectives++;
	}

	if(!SearchFile(in, "[NOUNS]"))
		return;
	while (GetLineFromFile(in, CurrentLine) > 1) {
		strcat(Nouns, CurrentLine);
		NumNouns++;
	}
	fclose(in);	// close in

	TauntInit = 1;	// Taunt initialized successfuly
}


//
// This is the function called by "cmd taunt,"
// it makes sure TauntInit is not 0, and outputs
// a random taunt using strings provided by GetTaunt
void Taunt(edict_t *ent)
{
	char *person;
	char taunt[MAX_LINE_SIZE];
	person = gi.args();	// get the argument list (the person to taunt)

	if (!TauntInit) {	// Taunt was not initialized
		gi.cprintf(ent, PRINT_HIGH, "Taunt has not initialized properly");
		return;
	}
	
	GetTaunt(taunt, VERB);	// read a verb
	// print the '"taunter" "verb" "taunted" with ' part of the message
	gi.bprintf(PRINT_MEDIUM, "%s %s %s with ", ent->client->pers.netname, taunt, person);
	
	GetTaunt(taunt, ADJECTIVE); // read an adjective
	// print the '" a/an adjective"' part
	gi.bprintf(PRINT_MEDIUM, "%s ", taunt);
	
	GetTaunt(taunt, NOUN); // get a noun
	// print the '"noun"!' part and add a newline
	gi.bprintf(PRINT_MEDIUM, "%s!\n", taunt);
}


//
// This reads a random element from pre-formatted
// elements[] (that contains 'num' elements) and stores
// it in s
void RandomTaunt(char s[], int num, char elements[])
{
	int i, j, RandNum;	
	RandNum = myrandom() % (num);		// This produces a random number between
										// 0 and num
	i = j = 0;
	while (j < RandNum) {				// This loop advances elements[]
		if (elements[i++] == ',')		// the the element just after
			j++;						// RandNum-1 commas have been passed
		
		else if (elements[i] == '\0') {	// If the character in this position is '\0'
			strcpy(s, "Error!");		// something has gone wrong, so fill s with
			return;						// "Error!" and return
		}
	}
	
	// this loop copies the selected element into s
	for (j = 0; j < MAX_LINE_SIZE-1 && elements[i] != ',' && elements[i] != '\0'; j++)
		s[j] = elements[i++];
	// add a '\0' at the end of s, so gi.bprintf() can print it
	// out properly
	s[j] = '\0';
}


//
// Uses RandomTaunt to store the proper element in s
void GetTaunt(char s[], int type)
{
	// this is self explanatory
	switch(type) {
	case VERB:
		RandomTaunt(s, NumVerbs, Verbs);
		break;
	
	case ADJECTIVE:
		RandomTaunt(s, NumAdjectives, Adjectives);
		break;
		
	case NOUN:
		RandomTaunt(s, NumNouns, Nouns);
		break;
	
	default:
		strcpy(s, "Error!");
		break;
	}
}


//
// This gets the next line in a file.
// It adds a comma to the end of the char *
// for purposes of adding to master array
int GetLineFromFile(FILE *in, char s[])
{
	int i, c;

	// This reads characters from in into s until MAX_LINE_SIZE-1 is reached,
	// a newline character is reached, or an EOF is reached.
	for (i = 0; i < MAX_LINE_SIZE-1 && (c = fgetc(in)) != '\n' && c != EOF; i++)
		s[i] = c;
	// Add a comma, this is necessary for proper array formatting
	s[i++] = ',';
	// Add a '\0' to the end of s
	s[i] = '\0';
	return i;
}


//
// This takes us to the line of the file right after
// 's' is found.
int SearchFile(FILE *in, char s[])
{
	char CurrentLine[MAX_LINE_SIZE];	// The line we are currently checking
	
	fseek(in, 0, SEEK_SET);	// move to beginning of file
	while (!feof(in)) {	// While we aren't at the end of in
		GetLineFromFile(in, CurrentLine);	// Store the current line into CurrentLine
		// Check to see if CurrentLine matches s
		if (strncmp(CurrentLine, s, strlen(s)-1) == 0)
			return 1;	// if it does, stop searching
	}
	return 0; // no match found
}

// fin mod : taunt
