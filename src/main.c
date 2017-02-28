#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "part.h"
#include "nn.h"

#define RATE 1.0
#define MIN(a,b) ((a)<(b)?(a):(b))
#define HIDDENSIZE	15
#define NOTESIZE		17
#define IPN	1
void fillinput(neuralnet* nn, note* n, int mode);
void train(neuralnet*nn, note* n);
note* parseoutput(neuralnet* nn);

int main(int argc, char* argv[]) {
	srand(time(NULL));
	int lsizes[3] = {HIDDENSIZE, NOTESIZE};
	neuralnet* nn = new_neuralnet(HIDDENSIZE+NOTESIZE,2,lsizes);
	part** parts = malloc((argc-1)*sizeof(part*));
	for(int i=0; i< argc-1;i++) {
		parts[i]=partFromFile(argv[i+1]);
		makeStatic(parts[i]);
		printToFile(parts[i], "testout.ly");
	}
	for(int g = 0; g<500;g++) {
		for(int p=0; p< argc-1;p++) {
			fillinput(nn,parts[p]->notes[0],2);
			for(int i=1;i<MIN(parts[p]->size,g/IPN+2);i++) {
				train(nn,parts[p]->notes[i]);
				fillinput(nn,parts[p]->notes[i-1],1);
			}
			update_weights(nn, RATE);
		}
	if(g%100 == 99) {
			printf("completed %d generations.\n",g+1);
		}
	}
	part *pout = newPart();
	note* n = newNote(0,0,2,4,0,0);
	pushNote(pout, n);
	fillinput(nn,n,2);
	for(int i =1; i< 200; i++) {
		pushNote(pout,parseoutput(nn));
		fillinput(nn,pout->notes[i-1],1);
	}
	printf("Filename: ");
	char* filename=malloc(100);
	scanf("%s", filename);
	printToFile(pout,filename);
	free(filename);
	for(int i=0; i<argc-1;i++) {
		destroyPart(parts[i]);
	}
	free(parts);
	destroyPart(pout);
	exit(EXIT_SUCCESS);
}
void fillinput(neuralnet* nn, note* n, int mode) {
	double *inputs = malloc((HIDDENSIZE+NOTESIZE)*sizeof(double));
	memset(inputs, 0, (HIDDENSIZE+NOTESIZE)*sizeof(double));
	if(mode == 1)
		memcpy(inputs, nn->layers[0].outputs, HIDDENSIZE*sizeof(double));
	if(mode == 2) {
		for(int i = 0; i< HIDDENSIZE;i++)
			inputs[i]=random_normal();
	}
	inputs[n->tone+HIDDENSIZE]=1;
	if(n->accidental==-1) 
		inputs[HIDDENSIZE+7]=1;
	else if(n->accidental==1)
		inputs[HIDDENSIZE+8]=1;
	switch(n->octave) {
		case 1:
			inputs[HIDDENSIZE+9]=1;
			break;
		case 2:
			inputs[HIDDENSIZE+10]=1;
			break;
		case 3:
			inputs[HIDDENSIZE+11]=1;
	}
	switch(n->duration) {
		case 2:
			inputs[HIDDENSIZE+12]=1;
			break;
		case 4:
			inputs[HIDDENSIZE+13]=1;
			break;
		case 8:
			inputs[HIDDENSIZE+14]=1;
	}
	if(n->dotted==1)
		inputs[HIDDENSIZE+15]=1;
	if(n->tie==1)
		inputs[HIDDENSIZE+16]=1;
	feedforward(nn, inputs);
	free(inputs);
}

void train(neuralnet*nn, note* n) {
	double *targets = malloc(NOTESIZE*sizeof(double));
	memset(targets, 0, NOTESIZE*sizeof(double));
	targets[n->tone]=1;
	if(n->accidental==-1)
		targets[7]=1;
	else if(n->accidental==1)
		targets[8]=1;
	switch(n->octave) {
		case 1:
			targets[9]=1;
			break;
		case 2:
			targets[10]=1;
			break;
		case 3:
			targets[11]=1;
	}
	switch(n->duration) {
		case 2:
			targets[12]=1;
			break;
		case 4:
			targets[13]=1;
			break;
		case 8:
			targets[14]=1;
	}
	if(n->dotted==1)
		targets[15]=1;
	if(n->tie==1)
		targets[16]=1;
	propagateback(nn,targets);
	free(targets);
}

note* parseoutput(neuralnet* nn) {
	int t=0;
	for(int i = 0; i<7;i++) {
		if(get_output(nn, nn->numlayers,t)<get_output(nn,nn->numlayers,i))
			t=i;
	}
	int a=0;
	int f=get_output(nn, nn->numlayers, 7);
	int s=get_output(nn, nn->numlayers, 8);
	if(f>0.8 || s>0.8)
		a=(f>s)?-1:1;
	int o=0;
	for(int i = 0; i<3;i++) {
		if(get_output(nn, nn->numlayers,o+9)<get_output(nn,nn->numlayers,i+9))
			o=i;
	}
	int d=0;
	for(int i = 0; i<3;i++) {
		if(get_output(nn, nn->numlayers,d+12)<get_output(nn,nn->numlayers,i+12))
			d=i;
	}
	int dot=0, tie=0;
	if(get_output(nn, nn->numlayers,15) > 0.8)
		dot=1;
	if(get_output(nn, nn->numlayers,16) > 0.8)
		tie=1;
	return newNote(t,a,o+1,pow(2,d+1), dot, tie);
}
