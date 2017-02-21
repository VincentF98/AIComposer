#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "part.h"

note* newNote(int tone, int octave, int duration) {
	note* n = malloc(sizeof(note));
	n->tone=tone;
	n->octave=octave;
	n->duration = duration;
	return n;
}

part* newPart() {
	part* p = malloc(sizeof(part));
	p->notes = malloc(72*sizeof(note*));
	p->maxsize = 72;
	p->size = 0;
	return p;
}

void pushNote(part* p, note* n) {
	if(p->size >= p->maxsize) { 
		p->notes = realloc(p->notes, (p->maxsize+24)*sizeof(note*));
		p->maxsize += 24;
	}
		p->notes[p->size++]=n;
}

note* popNote(part *p) {
	if(p->size == 0)
		return NULL;
	return p->notes[--p->size];
}

void destroyPart(part *p) {
	free(p->notes);
	free(p);
}

int charToTone(char c) {
	switch(c) {
		case 'c':
		case 'C': return 0;
		case 'd':
		case 'D': return 2;
		case 'e':
		case 'E': return 4;
		case 'f':
		case 'F': return 5;
		case 'g':
		case 'G': return 7;
		case 'a':
		case 'A': return 9;
		case 'b':
		case 'B': return 11;
		default: 							printf("ERROR: %c is not a valid note.\n",c);
													exit(EXIT_FAILURE);
	}
}

part* partFromFile(char* filename) {
	FILE *f = fopen(filename, "r");
	part *p = newPart();

	int t,o,d, c;
	t=-1;
	o=2;
	d=0;
	c = fgetc(f);

	while( c!= EOF) {
		while(c==' ' || c=='\n'){
			c = fgetc(f);
			printf(".");
		}
		if(c==EOF)
			break;
		printf("%c ",(char)c);
		o=2;
		d=0;
		t=charToTone((char)c);
		c=fgetc(f);
		while(c!= EOF && c!= ' ' && c != '\n') {
			switch((char) c) {
				case '#':
					t++;
					break;
				case 'b':
					if(t==0) {
						t=11;
						o--;
					}
					t--;
					break;
				case ',':
					o--;
					break;
				case '\'':
					o++;
					break;
				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9':
					d=10*d+c-(int)'0';
					break;
				default:
					printf("ERROR: invalid charakter: %c\n",(char)c);
					exit(EXIT_FAILURE);
			}
			c=fgetc(f);
		}
		pushNote(p,newNote(t,o,d));
	}
	fclose(f);
	printf("done reading\n");	
	return p;
}

void printToFile(part* p, char* filename) {
	FILE* f = fopen(filename, "w");
	fprintf(f, "\\version \"2.12.0\"\n");
	fprintf(f, "\t Stimme = {\\clef \"treble\" ");
	for(int i = 0; i<p->size; i++) {
		int n = p->notes[i]->tone;
		switch(n) {
			case 0:
			case 1:
				fputc('c',f);
				break;
			case 2:
			case 3:
				fputc('d',f);
				break;
			case 4:
				fputc('e',f);
				break;
			case 5:
			case 6:
				fputc('f',f);
				break;
			case 7:
			case 8:
				fputc('g',f);
				break;
			case 9:
			case 10:
				fputc('a',f);
				break;
			case 11:
				fputc('b',f);
		}
		if(n == 1 || n == 3 || n == 6 || n == 6 || n==8 || n==10)
			fputs("is",f);
		for(int o=2; o<p->notes[i]->octave;o++)
			fputc('\'',f);
		for(int o=p->notes[i]->octave; o < 2; o++)
			fputc(',',f);
		fprintf(f,"%d ", p->notes[i]->duration);
	}
	fprintf(f, " }\n");
	fprintf(f, "\\score {\n");
	fprintf(f, "\t<<\n\t\t<<\n");
	fprintf(f, "\t\t\t\\new Staff \\Stimme\n");
	fprintf(f, "\t\t>>\n\t>>\n");
	fprintf(f, "\t\\layout {\n");
	fprintf(f, "\t\t\\context {\n");
	fprintf(f, "\t\t\t\\Score\n");
	fprintf(f, "\t\t}\n\t}\n");
	fprintf(f, "\t\\midi {\n");
	fprintf(f, "\t\t\\context {\n");
	fprintf(f, "\t\t\t\\Score\n");
	fprintf(f, "\t\t\ttempoWholesPerMinute = #(ly:make-moment 200 4)\n");
	fprintf(f, "\t\t}\n\t}\n}");
	fclose(f);
}
