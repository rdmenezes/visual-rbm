# Tools #

This page contains an overview of the different command line tools available in the Tools.zip download found on the [Downloads](http://code.google.com/p/visual-rbm/wiki/Downloads?tm=2) page.

  * [Model Tools](Tools#Model_Tools.md)
    * [cltrain](Tools#cltrain.md)
    * [calchidden](Tools#calchidden.md)
    * [buildmlp](Tools#buildmlp.md)
  * [IDX Tools](Tools#IDX_Tools.md)
    * [idxinfo](Tools#idxinfo.md)
    * [catidx](Tools#catidx.md)
    * [joinidx](Tools#joinidx.md)
    * [splitidx](Tools#splitidx.md)
    * [csv2idx](Tools#csv2idx.md)
    * [idx2csv](Tools#idx2csv.md)
    * [image2csv](Tools#image2csv.md)
    * [shuffleidx](Tools#shuffleidx.md)


---


## Model Tools ##

### cltrain ###

Used to train a model from the command line.  Should be a bit faster than using the frontend, as there should be fewer GPU->CPU transfers (since there is no visualization to be done).  Quiet mode can also be specified which does not show the ongoing training error, and does not perform ANY GPU->CPU transfers.

#### Usage ####

```
cltrain [ARGS]
```

  * Required Arguments
    * -trainingData=**IDX** - Specifies the training data file.
    * -trainingLabels=**IDX** - Specifies the training label file (for MLP training only)
    * -schedule=**SCHEDULE** - Specifies the training schedule file (in the format specified on the [FileFormats](FileFormats.md) wiki page) to use during training.
    * -export=**OUT** - Specifies where to save the final model once training completes.
  * Optional Arguments
    * -validationData=**IDX** - Specifies an optional validation IDX dataset
    * -validationLabels=**IDX** - Specifies an optional validation label file (for MLP training only)
    * -import=**MODEL** - Specifies an initial set of weights to start from (rather than randomly initialization the weights).
    * -quiet - Suppresses all stdout output and omits any reconstruction error calculations.
    * -atlasSize=**SIZE** - Specifies the total memory allocated for our data atlas in megabytes.  Default value is 512.


---


### calchidden ###

Used to calculate the hidden activations of an RBM or AutoEncoder over an entire IDX dataset.  Alternatively, calculates the output activations of an MLP over an entire IDX dataset.

#### Usage ####

```
calchidden [INPUT] [OUTPUT] [RBM]
```

  * **INPUT** - The input IDX dataset.
  * **OUTPUT** - Destination to save the calculated hidden units.
  * **MODEL** - The trained model to use.


---


### buildmlp ###

Constructs a MLP from a stack of RBM, AutoEncoder, and MLP model files.

#### Usage ####

```
buildmlp [OPTIONS] files... -o [OUTPUT]
```

  * **OPTIONS**
    * **-f** - Adds subsequent models normally (default)
    * **-t** - Transpose subsequent models before adding (RBM and AutoEncoder only)
    * **-n INDEX COUNT** - Adds **COUNT** layers from an MLP starting at layer **INDEX** (MLP only)
  * **OUTPUT** - Filename to save new MLP model.

#### Examples ####
```
# constructs an autoencoder from a stack of 2 RBMs
buildmlp rbm1.json rbm2.json -t rbm2.json rbm1.json -o autoencoder.json

# pull out just the encoding half of above autoencoder
buildmlp -n 0 2 autoencoder.json -o encoder.json

# pull out just the decoding half of aboe autoencoder
buildmlp -n 2 2 autoencoder.json -o decoder.json
```

## IDX Tools ##

### idxinfo ###

Used to print an IDX file's header info (as described on the [FileFormats](FileFormats.md) wiki page).

#### Usage ####

```
idxinfo [INPUTS]
```

  * **INPUTS** - A list of IDX files whose header info you want to see.


---


### catidx ###

Used to concatanate together multiple datasets (with the same row dimensions and dataformat) into a single IDX dataset.  For instance, the MNIST training and test images could be concatanted into a single dataset with 70,000 length 784 rows; 60,000 rows from the training images and 10,000 rows from the test images.

#### Usage ####

```
catidx [INPUTS] [OUTPUT]
```

  * **INPUTS** - A list of IDX files to concatanate together.  All the given IDX files must have the same data format and row dimensions.
  * **OUTPUT** - Destination to save the concatanated inputs.


---


### joinidx ###

Used to join together multiple IDX datasets (with the same number of rows and dataformat) into a single IDX dataset.  For instance, the MNIST training images and training labels could be joined into a single dataset with 60,000 length 794 rows; 784 brightness values and 10 label values.

#### Usage ####
```
joinidx [INPUTS] [OUTPUT]
```

  * **INPUTS** - A list of IDX files to join together.  All the given IDX files must have the same data format and the same number of rows.
  * **OUTPUT** - Destination to save the joined inputs.


---


### splitidx ###

Used to take a subset of an IDX file and split it off into a separate, smaller IDX file.  For instance, splitidx could be used to divide an entire dataset into a training set and a testing set.

#### Usage ####
```
splitidx [INPUT] [OUTPUT] [FROM] [COUNT]
```

  * **INPUT** - The input IDX file we are generating a subset dataset from.
  * **OUTPUT** - The output IDX file to save.
  * **FROM** - The starting index in INPUT to start pulling rows from.
  * **COUNT** - Optional.  The number of rows to write to the OUTPUT file.  If no COUNT is given, then all the rows after the given FROM index will be written to OUTPUT.


---


### csv2idx ###

Used to convert a comma delimited text file to a single precision IDX data file for use with VisualRBM.  The input CSV is expected to not have a header.

#### Usage ####
```
csv2idx [INPUT] [OUTPUT]
```

  * **INPUT** - The CSV file to convert.
  * **OUTPUT** - The output IDX file to save.


---


### idx2csv ###

Prints an IDX data file to CSV.  CSV is either printed to file or to **stdout**.

#### Usage ####
```
idx2csv [INPUT] [OUTPUT]
```

  * **INPUT** - IDX file to convert.
  * **OUTPUT** - The output CSV file to save (optional).


---


### image2csv ###

Converts a single image to a comma delimited list of values (on [0,1]) printed to **stdout**.  Supports images in the following formats: JPEG, PNG, TGA, PSD, GIF, HDR, and PIC.

#### Usage ####
```
image2csv [INPUT]
```

  * **INPUT** - An image file to convert.


---


### shuffleidx ###

Shuffles an IDX file into a new random order.  Note, the PRNG used is always seeded with the same value so IDX files with the same length will always be sorted into identical random orders.

#### Usage ####
```
shuffleidx [INPUT] [OUTPUT]
```

  * **INPUT** - IDX file to shuffle.
  * **OUTPUT** - The output IDX file to save.