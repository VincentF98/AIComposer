#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "part.h"
#include "nn.h"

#define RATE 1.25
#define MIN(a,b) ((a)<(b)?(a):(b))
#define HIDDENSIZE	18
#define NOTESIZE		18
#define IPN	3
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
		printToFile(parts[i], "testout.ly");
	}
	for(int g = 0; g<300;g++) {
		for(int p=0; p< argc-1;p++) {
			fillinput(nn,parts[p]->notes[0],0);
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
	note* r = newNote(0,0,2,4,0,0);
	note* n = newNote(0,0,0,4,0,0);
	pout->relative=r;
	pushNote(pout, n);
	fillinput(nn,n,0);
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
	if(n->rest==1)
		inputs[HIDDENSIZE]=1;
	else
		inputs[n->tone+HIDDENSIZE+1]=1;
	if(n->accidental==-1) 
		inputs[HIDDENSIZE+8]=1;
	else if(n->accidental==1)
		inputs[HIDDENSIZE+9]=1;
	switch(n->octave) {
		case -1:
			inputs[HIDDENSIZE+10]=1;
			break;
		case 0:
			inputs[HIDDENSIZE+11]=1;
			break;
		case 1:
			inputs[HIDDENSIZE+12]=1;
	}
	switch(n->duration) {
		case 2:
			inputs[HIDDENSIZE+13]=1;
			break;
		case 4:
			inputs[HIDDENSIZE+14]=1;
			break;
		case 8:
			inputs[HIDDENSIZE+15]=1;
	}
	if(n->dotted==1)
		inputs[HIDDENSIZE+16]=1;
	if(n->tie==1)
		inputs[HIDDENSIZE+17]=1;
	feedforward(nn, inputs);
	free(inputs);
}

void train(neuralnet*nn, note* n) {
	double *targets = malloc(NOTESIZE*sizeof(double));
	memset(targets, 0, NOTESIZE*sizeof(double));
	if(n->rest==1)
		targets[0]=1;
	else
		targets[n->tone+1]=1;
	if(n->accidental==-1)
		targets[8]=1;
	else if(n->accidental==1)
		targets[9]=1;
	switch(n->octave) {
		case -1:
			targets[10]=1;
			break;
		case 0:
			targets[11]=1;
			break;
		case 1:
			targets[12]=1;
	}
	switch(n->duration) {
		case 2:
			targets[13]=1;
			break;
		case 4:
			targets[14]=1;
			break;
		case 8:
			targets[15]=1;
	}
	if(n->dotted==1)
		targets[16]=1;
	if(n->tie==1)
		targets[17]=1;
	propagateback(nn,targets);
	free(targets);
}

note* parseoutput(neuralnet* nn) {
	int t=0;
	for(int i = 0; i<8;i++) {
		if(get_output(nn, nn->numlayers,t)<get_output(nn,nn->numlayers,i))
			t=i;
	}
	int a=0;
	int f=get_output(nn, nn->numlayers, 8);
	int s=get_output(nn, nn->numlayers, 9);
	if(f>0.8 || s>0.8)
		a=(f>s)?-1:1;
	int o=0;
	for(int i = 0; i<3;i++) {
		if(get_output(nn, nn->numlayers,o+10)<get_output(nn,nn->numlayers,i+10))
			o=i;
	}
	int d=0;
	for(int i = 0; i<3;i++) {
		if(get_output(nn, nn->numlayers,d+13)<get_output(nn,nn->numlayers,i+13))
			d=i;
	}
	int dot=0, tie=0;
	if(get_output(nn, nn->numlayers,16) > 0.8)
		dot=1;
	if(get_output(nn, nn->numlayers,17) > 0.8)
		tie=1;
	return newNote(t-1,a,o-1,pow(2,d+1), dot, tie);
}
