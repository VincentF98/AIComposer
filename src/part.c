#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "part.h"
const int perfectUnison		= 0;
const int minorSecond 			= 1;
const int majorSecond			= 2;
const int minorThird				= 3;
const int majorThird				= 4;
const int perfectFourth		= 5;
const int augmentedFourth	= 6;
const int tritone					= 6;
const int diminishedFifth	= 6;
const int perfectFifth			= 7;
const int minorSixth				= 8;
const int majorSixth				= 9;
const int minorSeventh			= 10;
const int majorSeventh			= 11;
const int perfectOctave		= 12;

note* newNote(int tone, int accidental, int octave, int duration, int dotted, int tie) {
	note* n = malloc(sizeof(note));
	n->tone=tone;
	if(tone==-1)
		n->rest=1;
	else
		n->rest=0;
	n->accidental=accidental;
	n->octave=octave;
	n->duration = duration;
	n->dotted=dotted;
	n->tie=tie;
	return n;
}
note* copyNote(note* n) {
	return newNote(n->tone, n->accidental, n->octave, n->duration, n->dotted, n->tie);
}
part* newPart() {
	part* p = malloc(sizeof(part));
	p->notes = malloc(72*sizeof(note*));
	p->maxsize = 72;
	p->size = 0;
	p->relative=NULL;
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
	for(int i=0; i<p->size;i++)
		free(p->notes[i]);
	free(p->notes);
	free(p);
}

int charToTone(char c) {
	switch(c) {
		case 'c':
		case 'C': return 0;
		case 'd':
		case 'D': return 1;
		case 'e':
		case 'E': return 2;
		case 'f':
		case 'F': return 3;
		case 'g':
		case 'G': return 4;
		case 'a':
		case 'A': return 5;
		case 'b':
		case 'B': return 7;
		default: 							printf("ERROR: %c is not a valid note.\n",c);
													exit(EXIT_FAILURE);
	}
}
int accept(FILE *f, char* sym) {
	int l = strlen(sym);
	int i=0;
	char c=' ';
	while(i<l && c!=EOF) {
		c=fgetc(f);
		if(c!=sym[i++]) {
			fseek(f,(long)(-i),SEEK_CUR);
			return 0;
		}
	}
	if(i<l) {
		fseek(f,(long)(-i),SEEK_CUR);
		return 0;
	}
	//printf("accepted \"%s\"\n",sym);
	return 1;
}
note* acceptNote(FILE *f) {
	int t, a, o, d, dot, tie;
	d = 1;
	dot=0;
	tie=0;
	if(accept(f, "c"))	{
		t=0;
	}else if(accept(f, "d")) {
		t=1;
	}else if(accept(f, "e")) {
		t=2;
	}else if(accept(f, "f")) {
		t=3;
	}else if(accept(f, "g")) {
		t=4;
	}else if(accept(f, "a")) {
		t=5;
	}else if(accept(f, "b")) {
		t=6;
	}else if(accept(f, "r")) {
		t=-1;
	} else {
		return NULL;
	}

	if(accept(f, "is")) {
		a=1;
	}else if(accept(f, "es")) {
		a=-1;
	}else {
		a=0;
	}

	o=0;
	while(accept(f, ","))
		o--;
	while(accept(f, "'"))
		o++;

	if(accept(f, "128"))
		d=128;
	else if(accept(f, "64"))
		d=64;
	else if(accept(f, "32"))
		d=32;
	else if(accept(f, "16"))
		d=16;
	else if(accept(f, "8"))
		d=8;
	else if(accept(f, "4"))
		d=4;
	else if(accept(f, "2"))
		d=2;
	else if(accept(f, "1"))
		d=1;

	if(accept(f, "."))
		dot=1;

	if(accept(f, "~"))
		tie=1;
	//	printf("%d ", t);
	return newNote(t,a,o,d,dot,tie);
}
int acceptWhitespace(FILE* f) {
	int i = 0;
	char c=fgetc(f);
	while(c==' ' || c=='\t' || c== '\n') {
		i++;
		c=fgetc(f);
	}
	//printf("found %d whitespace charakters\n", i);
	fseek(f,(long)(-1),SEEK_CUR);
	return i;
}
void expect(FILE* f, char* sym) {
	if(accept(f,sym)!=1) {
		printf("ERROR: expected %s\n",sym);
		exit(EXIT_FAILURE);
	}
}
note* expectNote(FILE* f) {
	note* n=acceptNote(f);
	if(n==NULL) {
		printf("ERROR: expected note\n");
		exit(EXIT_FAILURE);
	}
	return n;
}
part* partFromFile(char* filename) {
	FILE *f = fopen(filename, "rb");
	part *p = newPart();
	acceptWhitespace(f);
	int relative = accept(f, "\\relative");
	note* r;
	if (relative == 1)
	{
		acceptWhitespace(f);
		r=expectNote(f);
		p->relative=r;
		acceptWhitespace(f);
		expect(f,"{");
		acceptWhitespace(f);
	}
	note* n =acceptNote(f);
	acceptWhitespace(f);
	while(n!=NULL) {
		pushNote(p,n);
		n=acceptNote(f);
		acceptWhitespace(f);
	}
	if(relative==1)
		expect(f,"}");
	fclose(f);
	printf("done reading %s\n", filename);	
	return p;
}
void makeStatic(part* p) {
	if(p->relative==NULL)
		return;
	int i=0;
	while(i<p->size && p->notes[i]->rest==1)
		i++;
	int intervall = p->notes[i]->tone-p->relative->tone;
	if(intervall > 4)
		p->notes[i]->octave+=p->relative->octave-1;
	else if(intervall < -4)
		p->notes[i]->octave+=p->relative->octave+1;
	else
		p->notes[i]->octave+=p->relative->octave;
	int li=i;
	for(i++; i<p->size; i++) {
		if(p->notes[i]->rest==0) {
			intervall=p->notes[i]->tone-p->notes[li]->tone;
			if(intervall > 4)
				p->notes[i]->octave+=p->notes[li]->octave-1;
			else if(intervall < -4)
				p->notes[i]->octave+=p->notes[li]->octave+1;
			else
				p->notes[i]->octave+=p->notes[li]->octave;
			li=i;
		}	
	}
	free(p->relative);
	p->relative=NULL;
}
void makeRelative(part* p, note* r) {
	if(p->relative!=NULL)
		makeStatic(p);
	p->relative=r;
	int i = 0;
	while(i < p->size && p->notes[i]->rest==1)
		i++;
	int intervall = p->notes[i]->tone-r->tone;
	int o, ob;
	o= p->notes[i]->octave;
	if(intervall > 4)
		p->notes[i]->octave-=r->octave-1;
	else if(intervall < -4)
		p->notes[i]->octave-=r->octave+1;
	else
		p->notes[i]->octave-=r->octave;
	int li=i;
	for(i++; i<p->size; i++) {
		if(p->notes[i]->rest==0) {
			intervall=p->notes[i]->tone-p->notes[li]->tone;
			ob=o;
			o=p->notes[i]->octave;
			if(intervall > 4)
				p->notes[i]->octave-=ob-1;
			else if(intervall < -4)
				p->notes[i]->octave-=ob+1;
			else
				p->notes[i]->octave-=ob;
			li=i;
		}
	}
}
int getPitch(note* n) {
	if(n->tone<=2)	//c, d and e
		return 2*n->tone+n->accidental;
	else						//f, g, a and b
		return 2*n->tone-1+n->accidental;
}
void fprintTone(FILE *f, note* n) {
	if(n->tone<=4)
		fputc((char)('c'+n->tone), f);
	else
		fputc((char)('a'+n->tone-5), f);
	if(n->accidental==-1)
		fprintf(f, "es");
	else if(n->accidental==1)
		fprintf(f, "is");
	if(n->octave<0){
		for(int o= n->octave;o<0;o++)
			fputc(',', f);
	}else if(n->octave>0) {
		for(int o= n->octave;o>0;o--)
			fputc('\'', f);
	}

}
void fprintNote(FILE* f, note* n) {
	if(n->rest==0)
		fprintTone(f,n);
	else
		fputc('r', f);
	fprintf(f, "%d", n->duration);
	if(n->dotted==1)
		fputc('.', f);
	if(n->tie==1)
		fputc('~', f);
	fputc(' ', f);
}
void printToFile(part* p, char* filename) {
	FILE* f = fopen(filename, "w");
	if(p->relative==NULL)
		makeRelative(p, copyNote(p->notes[0]));
	fprintf(f, "\\version \"2.12.0\"\n");
	fprintf(f, "Stimme = \\relative ");
	fprintTone(f,p->relative);
	fprintf(f, " {\n");
	fprintf(f, "\t\\tempo 4 = 150\n");
	fprintf(f, "\t\\clef \"treble\"\n");
	for(int i = 0; i<p->size; i++) {
		fprintNote(f, p->notes[i]);
	}
	fprintf(f, "\n}\n\n");
	fprintf(f, "\\score {\n");
	fprintf(f, "\\new Staff \\Stimme\n");
	fprintf(f, "\\layout { }\n");
	fprintf(f, "\\midi { }\n");
	fprintf(f, "}");
	fclose(f);
}
