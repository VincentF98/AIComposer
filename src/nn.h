#ifndef NN_H
#define NN_H
typedef struct {
	int size;
	double* inputs;
	double* outputs;
	double* deltas;
	double* biases;
	double* biasgradients;
	double* weights;
	double* weightgradients;
} layer;

typedef struct {
	int numinputs;
	int numlayers;
	int trainedsets;
	double* inputs;
	layer* layers;
} neuralnet;

neuralnet* new_neuralnet(int numinputs, int numlayers, int* layersizes);
int destroy_neuralnet(neuralnet* nn);
double get_bias(neuralnet* nn, int layer, int node);
int set_bias(neuralnet* nn, int layer, int node, double value);
double get_weight(neuralnet* nn, int layer1, int node1, int layer2, int node2);
int set_weight(neuralnet* nn, int layer1, int node1, int layer2, int node2, double value);
double get_input(neuralnet* nn, int layer, int node);
int set_input(neuralnet* nn, int layer, int node, double value);
double get_output(neuralnet* nn, int layer, int node);
int input_data(neuralnet* nn, double* data);
double sigmoid(double z);
int get_layersize(neuralnet* nn, int layer);
//double sigmoidprime(double z);

int feedforward(neuralnet* nn, double* inputs);
int propagateback(neuralnet* nn, double* targets);
int update_weights(neuralnet* nn, double rate);
#endif
