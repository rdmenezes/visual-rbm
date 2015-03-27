VisualRBM and associated tools makes use of three different filetypes:

  * [IDX (.idx)](https://code.google.com/p/visual-rbm/wiki/FileFormats#IDX) - binary data format used to store training data for VisualRBM
  * [Model (.json)](https://code.google.com/p/visual-rbm/wiki/FileFormats#Model) - JSON format used to store a model (RBM, AutoEncoder, etc) trained by VisualRBM
  * [Schedule (.json)](https://code.google.com/p/visual-rbm/wiki/FileFormats#Schedule) - JSON format used to represent a training schedule used by VisualRBM and cltrain


---


# IDX #

VisualRBM makes use of a slightly modified version of the IDX file-format as described at the bottom of [Yann LeCun's MNIST page](http://yann.lecun.com/exdb/mnist/).

| uint16\_t | Endianness |
|:----------|:-----------|
| uint8\_t | Data Format |
| uint8\_t | Dimensions |
| uint32\_t | Number of Elements in Dimension 1 |
| ... | ... |
| uint32\_t | Number of Elements in Dimension N |
| Data |

If Endianness is 0xFFFF, then all subsequent multi-byte data will be read with the assumption it is Little-Endian.  This includes the uint32\_t  'Number of Elements' fields.  If Endianness is 0x0000 (as described on Yan LeCun's page), then all subsequent data will be read in as Big Endian.  All other aspects of the format are identical.

The allowed data formats are:

| 0x08 | uint8\_t |
|:-----|:---------|
| 0x09 | int8\_t |
| 0x0B | int16\_t |
| 0x0C | int32\_t |
| 0x0D | single precision float ( 4 bytes) |
| 0x0E | double precision float ( 8 bytes) |

VisualRBM will only accept IDX files whose data format is a single precision floating point.

# Model #

Since version [r268](https://code.google.com/p/visual-rbm/source/detail?r=268), VisualRBM serializes trained models as a JSON object.  Each type of model has a different schema, but all models will have a string value called "Type" in the root object:

```

{
	"Type" : "ModelType",
	(Model Data)
}

```

Currently, the following models are supported:
  * [RestrictedBoltzmannMachine](https://code.google.com/p/visual-rbm/wiki/FileFormats#RestrictedBoltzmannMachine)
  * [AutoEncoder](https://code.google.com/p/visual-rbm/wiki/FileFormats#AutoEncoder)
  * [MultilayerPerceptron](https://code.google.com/p/visual-rbm/wiki/FileFormats#MultilayerPerceptron)

## RestrictedBoltzmannMachine ##

The RBM format is fairly simple and has the following format:

```

{
	"Type" : "RestrictedBoltzmannMachine",
	"VisibleCount" : ...,
	"HiddenCount" : ...,
	"VisibleType" : ...,
	"HiddenType" : ...,
	"VisibleBiases" : [...],
	"HiddenBiases" : [...],
	"Weights" : [[...],[...],...]
}

```

VisibleCount and HiddenCount must be positive non-zero integers.  VisibleType and HiddenType must each be one of:
  * Linear
  * Sigmoid
  * RectifiedLinear
  * Softmax
VisibleBiases and HiddenBiases are arrays of numbers representing the bias vectors.  Weights is a length HiddenCount array of length VisibleCount arrays of numbers representing the weights.

## AutoEncoder ##

The AutoEncoder's file format is simalar to the RBM:

```
{
	"Type" : "AutoEncoder",
	"VisibleCount" : ...,
	"HiddenCount" : ...,
	"HiddenType" : ...,
	"OutputType" : ...,
	"HiddenBiases" : [...],
	"OutputBiases" : [...],
	"Weights" : [[...],[...],...]
}
```

OutputType replaces VisibleType while OutputBiases replace VisibleBiases.  AutoEncoder supports all the same functions as the RBM.

## MultilayerPerceptron ##

The MLP format is a bit more complicated, since an MLP can have any number of layers.  MLPs have the following format:

```

{
	"Type" : "MultilayerPerceptron",
	"Layers" : [...]
}

```

Layers is an array of JSON objects which represent each layer of the MLP.  A layer ha sthe following format:

```

{
	"Inputs" : ...,
	"Outputs" : ...,
	"Function" : ...,
	"Biases" : [...],
	"Weights" : [[...],[...],...]
}

```

Inputs and Outputs must be positive non-zero integers.  Function must be one those mentioned above.  Biases is an array of numbers representing the output bias vector, while Weights is a length Outputs array of length Inputs arrays of numbers representing the weights.


---


# Schedule #

The training schedule allows the user to specify a model configuration and training parametrs (which can change over time).  A training schedule has the following general format:

```

{
	"Type" : ...,
	(Model Parameters)
	"Schedule" : 
	[
		{
			(Training Parameters),
			"Epochs" : ...
		},
		{
			(Training Parameters),
			"Epochs" : ...
		},
		...
	]
}
```

All training parameters default to 0.  For each object in the **Schedule** array, the **Epochs** parameter must be specified as a positive integer.  Some example training schedules follow:

### Example RBM Schedule ###
```
{
        "Type": "RBM",
	"VisibleCount" : 784,
        "HiddenCount": 100,
        "VisibleType": "Sigmoid",
        "HiddenType": "Sigmoid",
        "MinibatchSize": 10,
        "Schedule":
        [
                {
                        "LearningRate":0.01,
                        "Momentum":0.5,
                        "L1":0.0,
                        "L2":0.0,
                        "VisibleDropout":0.1,
                        "HiddenDropout":0.5,
                        "Epochs":10
                },
                {
                        "Momentum":0.6,
                        "Epochs":10
                },
                {
                        "Momentum":0.7,
                        "Epochs":10
                },
                {
                        "Momentum":0.8,
                        "Epochs":10
                },
                {
                        "Momentum":0.9,
                        "Epochs":60
                }
        ]
}
```

### Example AutoEncoder Schedule ###
```
{
        "Type": "AutoEncoder",
	"VisibleCount" : 784,
        "HiddenCount": 100,
        "OutputType": "Sigmoid",
        "HiddenType": "Sigmoid",
        "MinibatchSize": 10,
        "Schedule":
        [
                {
                        "LearningRate":0.01,
                        "Momentum":0.5,
                        "L1":0.0,
                        "L2":0.0,
                        "VisibleDropout":0.1,
                        "HiddenDropout":0.5,
                        "Epochs":10
                },
                {
                        "Momentum":0.6,
                        "Epochs":10
                },
                {
                        "Momentum":0.7,
                        "Epochs":10
                },
                {
                        "Momentum":0.8,
                        "Epochs":10
                },
                {
                        "Momentum":0.9,
                        "Epochs":60
                }
        ]
}
```

### Example MLP Schedule ###

MLP schedules are a bit more complicated and powerful than those for single layer models.  Each parameter training parameter can optionally be specified on a per layer level.

For instance, the following schedule specifies dropout probability of 0.1 on the input layer, 0.5 on the first hidden layer, and 0.5 on the second hidden layer.  Likewise, unit accumulation Noise has been verbosely set to have standard deviation 0.0 for the first hidden layer, the second hidden layer, and the output layer.

```
{
	"Type" : "MLP",
	"Layers" : [784, 500, 500, 10],
	"ActivationFunctions" : ["Sigmoid", "Sigmoid", "Softmax"],
	"MinibatchSize" : 10,
	"Schedule":
	[
		{
			"LearningRate" : 0.05,
			"Momentum" : 0.5,
			"L1" : 0.0,
			"L2" : 0.0,
			"Dropout" : [0.1, 0.5, 0.5],
			"Noise" : [0.0, 0.0, 0.0],
			"Epochs" : 10
		},
		{
			"Momentum" : 0.6,
			"Epochs" : 10
		},
		{
			"Momentum" : 0.7,
			"Epochs" : 10
		},
		{
			"Momentum" : 0.8,
			"Epochs" : 10
		},
		{
			"Momentum" : 0.9,
			"Epochs" : 60
		}
	]	
}
```