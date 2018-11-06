#include <execinfo.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "nn.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void print_trace ()
{
  void *array[10];
  size_t size;
  char **strings;
  size_t i;

  size = backtrace (array, 10);
  strings = backtrace_symbols (array, size);

  printf ("Obtained %zd stack frames.\n", size);

  for (i = 0; i < size; i++)
     printf ("%s\n", strings[i]);

  free (strings);
}

int check_layer(neuralnet* nn, int layer) {
	if( layer <= nn->numlayers ) {
		if(layer >= 0)
			return 1;
		printf("Layer can not be negative.\n");
		print_trace();
		exit(EXIT_FAILURE);
	}
	printf("There are only %d layers.\n", nn->numlayers + 1);
	print_trace();
	exit(EXIT_FAILURE);
}

int check_node(neuralnet* nn, int layer, int node) {
	check_layer(nn, layer);
	if(node < get_layersize(nn, layer)) {
		if(node >= 0)
			return 1;
		printf("Node can not be negative.\n");
		print_trace();
		exit(EXIT_FAILURE);
	}
	printf("Layer %d has only %d nodes.\n", layer, get_layersize(nn, layer));
	print_trace();
	exit(EXIT_FAILURE);
}
double drand()   /* uniform distribution, (0..1] */
{
  return (rand()+1.0)/(RAND_MAX+1.0);
}

double random_normal() 
 /* normal distribution, centered on 0, std dev 1 */
{
  return sqrt(-2*log(drand())) * cos(2*M_PI*drand());
}

neuralnet* new_neuralnet(int numinputs, int numlayers, int* layersizes) {
	neuralnet* nn = malloc(sizeof(neuralnet));
	nn->numinputs = numinputs;
	nn->numlayers = numlayers;
	nn->inputs = malloc(numinputs*sizeof(double));
	nn->layers = malloc(numlayers*sizeof(layer));
	for(int l = 0; l < numlayers; l++) {
		nn->layers[l].size = layersizes[l];
		nn->layers[l].inputs = malloc(layersizes[l]*sizeof(double));
		nn->layers[l].outputs = malloc(layersizes[l]*sizeof(double));
		nn->layers[l].deltas = malloc(layersizes[l]*sizeof(double));
		nn->layers[l].biases = malloc(layersizes[l]*sizeof(double));
		nn->layers[l].biasgradients = malloc(layersizes[l]*sizeof(double));
		int numweights;
		if(l==0)
			numweights = numinputs*layersizes[0];
		else
			numweights = layersizes[l-1]*layersizes[l];
		nn->layers[l].weights=malloc(numweights*sizeof(double));
		nn->layers[l].weightgradients = malloc(numweights*sizeof(double));
		memset(nn->layers[l].weightgradients, 0, numweights*sizeof(double));
		memset(nn->layers[l].biasgradients, 0, layersizes[l]*sizeof(double));
		for(int i = 0;i < layersizes[l];i++) {
			nn->layers[l].biases[i] = random_normal();
		}
		for(int i = 0;i < numweights;i++) {
			nn->layers[l].weights[i] = random_normal();
		}
	}
	return nn;
}

void destroy_neuralnet(neuralnet* nn) {
	for(int l = 0; l < nn->numlayers; l++) {
		free(nn->layers[l].inputs);
		free(nn->layers[l].outputs);
		free(nn->layers[l].deltas);
		free(nn->layers[l].biases);
		free(nn->layers[l].biasgradients);
		free(nn->layers[l].weights);
		free(nn->layers[l].weightgradients);
	}
	free(nn->layers);
	free(nn->inputs);
	free(nn);
}


double get_bias(neuralnet* nn, int layer, int node) {
	check_node(nn, layer, node);
	return nn->layers[layer-1].biases[node];
}
double get_bias_gradient(neuralnet* nn, int layer, int node) {
	check_node(nn, layer, node);
	return nn->layers[layer-1].biasgradients[node];
}
void set_bias(neuralnet* nn, int layer, int node, double value) {
	check_node(nn, layer, node);
	nn->layers[layer-1].biases[node] = value;
}
void set_bias_gradient(neuralnet* nn, int layer, int node, double value) {
	check_node(nn, layer, node);
	nn->layers[layer-1].biasgradients[node] = value;
}
double get_weight(neuralnet* nn, int layer1, int node1, int layer2, int node2) {
	check_node(nn, layer1, node1);
	check_node(nn, layer2, node2);
	int l2size = nn->layers[layer2-1].size;
	return nn->layers[layer2-1].weights[l2size*node1+node2];
}
double get_weight_gradient(neuralnet* nn, int layer1, int node1, int layer2, int node2) {
	check_node(nn, layer1, node1);
	check_node(nn, layer2, node2);
	int l2size = nn->layers[layer2-1].size;
	return nn->layers[layer2-1].weightgradients[l2size*node1+node2];
}
void set_weight(neuralnet* nn, int layer1, int node1, int layer2, int node2, double value) {
	check_node(nn, layer1, node1);
	check_node(nn, layer2, node2);
	int l2size = nn->layers[layer2-1].size;
	nn->layers[layer2-1].weights[l2size*node1+node2] = value;
}
void set_weight_gradient(neuralnet* nn, int layer1, int node1, int layer2, int node2, double value) {
	check_node(nn, layer1, node1);
	check_node(nn, layer2, node2);
	int l2size = nn->layers[layer2-1].size;
	nn->layers[layer2-1].weightgradients[l2size*node1+node2] = value;
}
double get_delta(neuralnet* nn, int layer, int node) {
	check_node(nn, layer, node);
	return nn->layers[layer-1].deltas[node];
}

void set_delta(neuralnet* nn, int layer, int node, double value) {
	check_node(nn, layer, node);
	nn->layers[layer-1].deltas[node] = value;
}

double get_input(neuralnet* nn, int layer, int node) {
	check_node(nn, layer, node);
	if(layer == 0)
		return nn->inputs[node];
	else
		return nn->layers[layer-1].inputs[node];
}

void set_input(neuralnet* nn, int layer, int node, double value) {
	check_node(nn, layer, node);
	if(layer == 0)
		nn->inputs[node] = value;
	else
		nn->layers[layer-1].inputs[node] = value;
}

double get_output(neuralnet* nn, int layer, int node) {
	check_node(nn, layer, node);
	if(layer == 0)
		return nn->inputs[node];
	else
		return nn->layers[layer-1].outputs[node];
}

int get_layersize(neuralnet* nn, int layer) {
	check_layer(nn, layer);
	if(layer == 0)
		return nn->numinputs;
	return nn->layers[layer-1].size;
}

double sigmoid(double z) {
	return 1.0/(1.0+exp(-z));
}

/*double sigmoidprime(double z) {
	double s=sigmoid(z);
	return s*(1-s);
}*/

void input_data(neuralnet* nn, double* data) {
	for(int i = 0; i < nn->numinputs; i++) {
		nn->inputs[i]=data[i];
	}
}
double calculate_output(neuralnet* nn, int layer, int node) {
	if(layer == 0)
		return nn->inputs[node];
	int numinputs;
	if(layer == 1)
		numinputs = nn->numinputs;
	else
		numinputs = nn->layers[layer-2].size;
	double sum=0;
	for(int n = 0; n < numinputs;n++) {
		sum += get_weight(nn, layer-1, n, layer, node)*calculate_output(nn, layer-1,n);
	}
	set_input(nn, layer, node, sum);
	double output = sigmoid(sum+get_bias(nn, layer, node));
	nn->layers[layer-1].outputs[node] = output;
	return output;
}


void feedforward(neuralnet* nn, double* inputs) {
	input_data(nn, inputs);
	int outputsize = get_layersize(nn, nn->numlayers);
	calculate_output(nn, nn->numlayers, 0);
	
	for(int i = 1; i< outputsize; i++) {
		double sum = 0;
		for(int n = 0; n < get_layersize(nn, nn->numlayers - 1); n++) {
			sum += get_weight(nn, nn->numlayers-1, n, nn->numlayers, i)*get_output(nn, nn->numlayers-1,n);
		}
		double output = sigmoid(sum + get_bias(nn, nn->numlayers, i));
		nn->layers[nn->numlayers-1].outputs[i] = output;
		
	}
	
}

void propagateback(neuralnet* nn, double* targets) {
	//output layer
	int l = nn->numlayers;
	int lsize = get_layersize(nn,l);
	for(int n = 0; n < lsize ; n++) {
		double out = get_output(nn, l, n);
		double delta = -(targets[n]-out)*out*(1-out);
		set_delta(nn,l,n, delta );
	}
	//hidden layers
	for(l=nn->numlayers-1;l>0;l--) {
		int l1size = get_layersize(nn,l);
		int l2size = get_layersize(nn,l+1);
		for(int n1 = 0; n1 < l1size; n1++) {
			double delta = 0;
			for(int n2 = 0; n2 < l2size; n2++) {
				delta += get_delta(nn, l+1, n2)*get_weight(nn, l,n1,l+1,n2);
			}
			double out = get_output(nn, l, n1);
			delta = delta*out*(1-out);
			set_delta(nn, l, n1, delta);
		}
	}

	for(l = 0; l < nn->numlayers; l++) {
		int l1size = get_layersize(nn,l);
		int l2size = get_layersize(nn,l+1);
		//update weights
		for(int n1 = 0; n1 < l1size; n1++) {
			for(int n2 = 0; n2 < l2size; n2++) {
				double gr = get_weight_gradient(nn, l, n1, l+1, n2);
				double delta = get_delta(nn, l+1, n2);
				double out = get_output(nn, l, n1);
				set_weight_gradient(nn, l, n1, l+1, n2, gr+delta*out);
			}
		
		}
		
		for(int n2 = 0; n2 < l2size; n2++) {
			// update bias
			double gr = get_bias_gradient(nn, l+1, n2);
			double delta = get_delta(nn, l+1, n2);
			set_bias_gradient(nn, l+1, n2, gr+delta);
		}
		
	}
	nn->trainedsets++;
	
}

void update_weights(neuralnet* nn, double rate) {
	for(int l = 0; l < nn->numlayers; l++) {
		int l1size = get_layersize(nn,l);
		int l2size = get_layersize(nn,l+1);
		//update weights
		for(int n1 = 0; n1 < l1size; n1++) {
			for(int n2 = 0; n2 < l2size; n2++) {
				double gr = get_weight_gradient(nn, l, n1, l+1, n2)/nn->trainedsets;
				set_weight_gradient(nn, l, n1, l+1, n2, 0.0);
				double cw = get_weight(nn, l, n1, l+1, n2);
				set_weight(nn, l, n1, l+1, n2, cw-rate*gr);
			}
			
		
		}
		for(int n2 = 0; n2 < l2size; n2++) {
			// update bias
			double gr = get_bias_gradient(nn, l+1, n2)/nn->trainedsets;
			set_bias_gradient(nn, l+1, n2, 0.0);
			double cb = get_bias_gradient(nn, l+1, n2);
			set_bias(nn, l+1, n2, cb-rate*gr);
		}
	}
	nn->trainedsets=0;
}

