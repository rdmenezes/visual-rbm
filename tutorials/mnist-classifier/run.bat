:: Initializes a 784-500-500-10 network with:
::  784-500 RBM with Sigmoid Visible Units and Rectified Linear Hidden Units
::  500-500 RBM with Rectified Linear Visible and Hiddden Units
::  500-10 single layer MLP with Softmax output
:: After initialization, the network is fine tuned with backpropagation
:: Finally, the network's estimates for the labels on the test set are calculated and saved to test-estimates.idx

:: train the first layer
cltrain -trainingData=mnist-train-images.idx -schedule=layer1-schedule.json -export=layer1.json

:: calculate the first layer of hidden activations
calchidden mnist-train-images.idx hidden1.idx layer1.json

:: train the second layer
cltrain -trainingData=hidden1.idx -schedule=layer2-schedule.json -export=layer2.json

:: calculate the second layer of hidden activations
calchidden hidden1.idx hidden2.idx layer2.json

:: now initialize the last layer with single layer MLP with softmax output units
cltrain -trainingData=hidden2.idx -trainingLabels=mnist-train-labels.idx -schedule=layer3-schedule.json -export=layer3.json

:: stick all our layers together into an MLP
buildmlp layer1.json layer2.json layer3.json -o mlp.json

:: now fine tune our classifier with backpropagation
cltrain -trainingData=mnist-train-images.idx -trainingLabels=mnist-train-labels.idx -schedule=mlp-schedule.json -import=mlp.json -export=mlp-tuned.json

:: export our label estimates for the test set for later evaluation
calchidden mnist-test-images.idx test-estimates.idx mlp-tuned.json