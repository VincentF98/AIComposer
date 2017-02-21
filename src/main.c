#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "part.h"
#include "nn.h"

#define RATE 1.0
#define MIN(a,b) ((a)<(b)?(a):(b))
#define MEMORY (-8)
#define IPN	1
void fillinput(neuralnet* nn, note* n, int mode);
void train(neuralnet*nn, note* n);
note* parseoutput(neuralnet* nn);

int main(int argc, char* argv[]) {
	srand(time(NULL));
	int lsizes[3] = {18+MEMORY ,18};
	neuralnet* nn = new_neuralnet(2*18+MEMORY,2,lsizes);
	part** parts = malloc((argc-1)*sizeof(part*));
	for(int i=0; i< argc-1;i++)
		parts[i]=partFromFile(argv[i+1]);
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
	note* n = newNote(0,3,4);
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
	double *inputs = malloc((2*18+MEMORY)*sizeof(double));
	memset(inputs, 0, (2*18+MEMORY)*sizeof(double));
	if(mode == 1)
		memcpy(inputs, nn->layers[0].outputs, (18+MEMORY)*sizeof(double));
	if(mode == 2) {
		for(int i = 0; i< 18+MEMORY;i++)
			inputs[i]=random_normal();
	}
	inputs[n->tone+18+MEMORY]=1;
	switch(n->octave) {
		case 1:
			inputs[18+MEMORY+12]=1;
			break;
		case 2:
			inputs[18+MEMORY+12+1]=1;
			break;
		case 3:
			inputs[18+MEMORY+12+2]=1;
	}
	switch(n->duration) {
		case 2:
			inputs[18+MEMORY+12+3]=1;
			break;
		case 4:
			inputs[18+MEMORY+12+4]=1;
			break;
		case 8:
			inputs[18+MEMORY+12+5]=1;
	}
	feedforward(nn, inputs);
	free(inputs);
}

void train(neuralnet*nn, note* n) {
	double *targets = malloc(18*sizeof(double));
	memset(targets, 0, 18*sizeof(double));
	targets[n->tone]=1;
	switch(n->octave) {
		case 1:
			targets[12]=1;
			break;
		case 2:
			targets[13]=1;
			break;
		case 3:
			targets[14]=1;
	}
	switch(n->duration) {
		case 2:
			targets[15]=1;
			break;
		case 4:
			targets[16]=1;
			break;
		case 8:
			targets[17]=1;
	}
	propagateback(nn,targets);
	free(targets);
}

note* parseoutput(neuralnet* nn) {
	int t=0;
	for(int i = 0; i<12;i++) {
		if(get_output(nn, nn->numlayers,t)<get_output(nn,nn->numlayers,i))
			t=i;
	}
	int o=0;
	for(int i = 0; i<3;i++) {
		if(get_output(nn, nn->numlayers,o+12)<get_output(nn,nn->numlayers,i+12))
			o=i;
	}
	int d=0;
	for(int i = 0; i<3;i++) {
		if(get_output(nn, nn->numlayers,d+15)<get_output(nn,nn->numlayers,i+15))
			d=i;
	}
	return newNote(t,o+1,pow(2,d+1));
}
