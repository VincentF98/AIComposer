#ifndef PART_H
#define PART_H

typedef struct {
	int tone;
	int octave;
	int duration;
} note;

typedef struct {
	note** notes;
	int maxsize;
	int size;
} part;

double random_normal();
note* newNote(int tone, int octave, int duration);
part* newPart();
void pushNote(part* p, note *n);
note *popNote(part* p);
void destroyPart(part* p);
part* partFromFile(char* filename);
void printToFile(part* p, char* filename);

#endif
