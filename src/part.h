#ifndef PART_H
#define PART_H

typedef struct {
	int tone;
	int rest;
	int accidental;
	int octave;
	int duration;
	int dotted;
	int tie;
} note;

typedef struct {
	note** notes;
	note* relative;
	int maxsize;
	int size;
} part;

double random_normal();
note* newNote(int tone, int accidental, int octave, int duration, int dotted, int tie );
part* newPart();
void pushNote(part* p, note *n);
note *popNote(part* p);
void destroyPart(part* p);
part* partFromFile(char* filename);
void printToFile(part* p, char* filename);
int getPitch(note* n);
void makeStatic(part* p);
void makeRelative(part* p, note *r);

#endif
