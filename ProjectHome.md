
---


| <h3><a href='FAQ.md'>FAQ</a></h3> | <h3><a href='Screenshots.md'>Screenshots</a></h3> | <h3><a href='http://groups.google.com/group/visual-rbm'>Forum</a></h3> | <h3><a href='https://trello.com/board/visualrbm/4f66455669944e074913f564'>Project Roadmap</a></h3> |     <h3><a href='https://visual-rbm.googlecode.com/svn/trunk/source/Web/IDXView.xhtml'>IDX Browser</a> <table><thead><th> <h3><a href='https://code.google.com/p/visual-rbm/wiki/Downloads?ts=1402983237&updated=Downloads#Tutorials'>Tutorials</a></h3> </th></thead><tbody></tbody></table>

<hr />

<h1>Visual RBM Training Tool</h1>

VisualRBM is an interactive application for quickly training Restricted Boltzmann Machines (RBM) and AutoEncoders with tied weights.  Users can keep track of training progress by visualizing the visible input and reconstructions, the hidden activation probabilities, and the feature detectors while training occurs on the GPU.  Learning rate, momentum, regularization and dropout parameters can all be controlled and tweaked at runtime.<br>
<br>
VisualRBM is still in development at this point, so bug reports would be greatly appreciated.<br>
<br>
<hr />

<h1>News</h1>

<h3>1/10/2015</h3>
No new releases recently (and none coming for awhile) but don't worry, I am hard at work.  I'm doing some major work on the GPU backend library, SiCKL ( <a href='https://code.google.com/p/sickl/'>https://code.google.com/p/sickl/</a> ):<br>
<ul><li>Changed SiCKL to use qmake so I can port to OSX and Linux<br>
</li><li>Implementing an OpenCL frontend for SiCKL<br>
</li><li>Making SiCKL a proper 64-bit shared library</li></ul>

This work is ongoing and is probably going to take awhile, but once I've finished I'll be able to easily port the command-line tools to other operating systems (targeting OSX and Linux).  The eventual goal is to completely port VisualRBM to Qt on Windows, OSX and Linux and the third of you that come here on operating systems over than Windows will be able to leave happy :)<br>
<br>
<h3>8/31/2014 - <a href='http://goo.gl/gcLnAI'>VisualRBM 1.2</a></h3>
<ul><li>fixed issue in model JSON parsing and serialization code where parsing or serializing large models would cause crash<br>
<ul><li>cJSON replaced with new incremental JSON parser/serializer for models</li></ul></li></ul>

<h3>8/9/2014 - <a href='http://goo.gl/FCXO1I'>VisualRBM 1.1</a></h3>
<ul><li>fixed bug in installer where joinidx.exe was not being installed properly</li></ul>

<h3>8/05/2014 - <a href='http://goo.gl/ZPd2Si'>VisualRBM 1.0</a></h3>
<ul><li>created a snazzy NSIS installer for VisualRBM + Tools all in one package<br>
<ul><li>binary directory is automatically added to PATH variable<br>
</li></ul></li><li>added a <a href='https://visual-rbm.googlecode.com/svn/trunk/helpers/IDX.py'>Python IDX class</a> for manipulating VisualRBM's IDX data files<br>
</li><li>added support for <a href='http://arxiv.org/abs/1212.5701'>Adadelta</a> learning with tweakable decay parameter to RBM, AutoEncoder, and MLP trainers<br>
</li><li>replaced momentum with <a href='http://www.cs.toronto.edu/~fritz/absps/momentum.pdf'>Nesterov accelerated gradient</a> to RBM, AutoEncoder and MLP trainers<br>
</li><li>refactored dropout code in RBM and AutoEncoder to be slightly more efficient<br>
<ul><li>however, black speckles indicating which units have been dropped out are no longer visible<br>
</li></ul></li><li>added support for specifying random seed used to generate weights and other stochastic data<br>
<ul><li>prior to this version, the default seed was 1<br>
</li></ul></li><li>added optional -atlasSize command-line parameter to cltrain and VisualRBM which allows users to specify the allocation size for space on GPU used to store training data<br>
<ul><li>users experiencing crash with VisualRBM shortly after running should try running from command prompt "VisualRBM -atlasSize=VAL" with various values less than 512 until VisualRBM stops crashing on run.  Once this number has been determined, update your VisualRBM shortcut to match this comamnd string.<br>
</li><li>default value is (and was) 512<br>
</li></ul></li><li>fixed bug in error calculation where Softmax was not being calculated correctly<br>
</li><li>fixed crash bug in VisualRBM when user would load a second schedule after first schedule had completed</li></ul>

<h3>6/16/2014 - <a href='http://goo.gl/2QLeZZ'>VisualRBM Build r333</a>, <a href='http://goo.gl/0EEyw0'>Tools Build r333</a></h3>
<ul><li>added a <a href='https://code.google.com/p/visual-rbm/wiki/Downloads?ts=1402983237&updated=Downloads#Tutorials'>Tutorial</a> section to the Downloads page which includes a zip containing a script and schedule files for training a classifier for the MNIST dataset<br>
</li><li>added MLP training support to 'cltrain' command-line tool<br>
</li><li>added new command-line tool 'buildmlp' which can construct an MLP model from a stack of RBMs, AutoEncoders, and MLPs<br>
</li><li>added support to all training algorithms for 'Softmax' activation function<br>
</li><li>added support to calchidden for handling models using 'Softmax' activation function<br>
</li><li>fixed bug in SiCKL shader code generation which serialized float values without enough precision<br>
<ul><li>fixes bug in PRNG NextFloat() functions<br>
</li><li>fixes crash bug on some AMD cards where 0 would be serialized as '0f' which isn't valid and prevented shader compilation<br>
</li></ul></li><li>fixed bug in AutoEncoder trainer where weight scaling due to Hidden dropout was calculated incorrectly (so AutoEncoder's should work much better now)<br>
</li><li>fixed bug in DataAtlas where 'streaming' flag was not being reset on Initialize<br>
<ul><li>in VisualRBM if you loaded a large dataset that could not all fit in video RAM, did some training, and then loaded a much smaller dataset, the small dataset would get copied multiples times into video RAM and streamed in again and again during training<br>
</li></ul></li><li>fixed minor bug in VisualRBM where error logging would switch to scientific notation once error got sufficiently small which wasn't terrible readable</li></ul>

<h3>4/29/2014 - <a href='http://goo.gl/j8IIUg'>VisualRBM Build r309</a>, <a href='http://goo.gl/mK4G6Z'>Tools Build r309</a></h3>
<ul><li>replaced 32 bit LCG PRNG with 128 bit XORShift128 PRNG<br>
<ul><li>All random number streams should be pretty much guaranteed to be uncorrelated<br>
</li></ul></li><li>fixed bug in AutoEncoder parsing logic where the Hidden and Output Activation functions would get swapped<br>
</li><li>fixed bug where memory was not 16 byte aligned in calchidden, causing crash<br>
</li><li>fixed bug where last feature would not get rendered when training an AutoEncoder<br>
</li><li>fixed bug where Export button was enabled after Importing a model (pressing Export without Starting/Pausing first would cause crash))<br>
</li><li>fixed crash caused by GPU memory fragmentation<br>
<ul><li>was triggered when changing the the minibatch size, or dataset used<br>
</li><li>would cause DataAtlas to re-allocate GPU memory, now DataAtlas allocates a single large block (384 MB) and reuses it for lifetime of VisualRBM</li></ul></li></ul>

<h3>3/15/2014 - <a href='http://visual-rbm.googlecode.com/svn-history/r293/trunk/release/VisualRBM.zip'>VisualRBM Build r293</a>, <a href='http://visual-rbm.googlecode.com/svn-history/r293/trunk/release/Tools.zip'>Tools Build r293</a></h3>
<ul><li>Added support for actual AutoEncoder class, which uses tied encoder/decoder weights trained with BackPropagation<br>
<ul><li>AutoEncoder serialization identical to RestrictedBoltzmannMachine serialization, except type is 'AutoEncoder'<br>
</li></ul></li><li>Reduced max-size of DataAtlas to 384 mb (down from 512 mb)<br>
<ul><li>OOM crash was occurring for me on some datasets<br>
</li></ul></li><li>Added command-line tool 'image2csv' which converts an image to serialized CSV format (uses stb_image.c library)<br>
</li><li>Added command-line tool 'shuffleidx' which shuffles an IDX file into a new random order</li></ul>

<h3>2/12/2014 - <a href='http://visual-rbm.googlecode.com/svn-history/r268/trunk/release/VisualRBM.zip'>VisualRBM Build r268</a>, <a href='http://visual-rbm.googlecode.com/svn-history/r268/trunk/release/Tools.zip'>Tools Build r268</a></h3>
<ul><li>Integrated new OMLT backend (which is much easier to embed)<br>
</li><li>Added initial support for AutoEncoders trained with backpropagation (encode/decode weights not currently tied)<br>
</li><li>Changed model serialization to new JSON format<br>
</li><li>Added abililty to create a JSON training schedule which can be used to automatically change training parameters during training<br>
</li><li>Added support for Rectified Linear activation function to both RBM and AutoEncoder<br>
</li><li>Amount of time delay before querying latest visible/hidden/weights dependent on how many we are visualizing<br>
</li><li>Fixed bug where very large reconstruction error values would destroy the error graph</li></ul>

<h3>7/21/2013</h3>

<ul><li>Implemented a simple 'infinite scroll' IDX viewer in JavaScript and HTML5 (tested in Chrome)<br>
</li><li>Latest version can be found via the 'IDX Browser' link at the top or this URL: <a href='https://visual-rbm.googlecode.com/svn/trunk/source/Web/IDXView.xhtml'>https://visual-rbm.googlecode.com/svn/trunk/source/Web/IDXView.xhtml</a>
</li><li>Click and drag the data to navigate (will be adding arrow keys and page up/down soon)<br>
</li><li>If you're interested in your own visualizations, IDX.js file reader can be found in that same source directory as well.</li></ul>

<h3>7/7/2013</h3>
<blockquote>No new releases recently, but a fair bit of work is being done behind the scenes.  I've been working on implementing BackPropagation (with dropout!) with SiCKL and an associated MultilayerPerceptron class.  If you've been following the source check-ins you will notice I've added a new VS Solution OMLT.  That branch will be in a state of flux for awhile and will eventually completely replace the 'QuickBoltzmann' project in VisualRBM and clrbm.</blockquote>

<blockquote>For the end users and developers here's what's coming down the line:<br>
</blockquote><ul><li>OMLT is properly intended for embedding, so developers should have an easier time using the backend lib.<br>
</li><li>OMLT includes a 'DataAtlas' class which takes care of streaming in rows of data from IDX data files, and converting them to the required SiCKL format used by the BackPropagation and RBMTrainer classes.  The data is also now loaded into a single large texture, rather than thousands of little ones.  As a result, GPU memory fragmentation is vastly reduced (MNIST training now uses about 200 MB of video memory now!)<br>
</li><li>MultilayerPerceptron and eventually RestrictedBoltzmannMachine will be serialized in a JSON format (rather than the current ad-hoc binary format)</li></ul>


<h3>4/27/2013 - <a href='http://visual-rbm.googlecode.com/svn-history/r168/trunk/release/VisualRBM.zip'>VisualRBM Build r168</a>, <a href='http://visual-rbm.googlecode.com/svn-history/r168/trunk/release/Tools.zip'>Tools Build r168</a></h3>
<ul><li>Fixed weight update shader to not perform weight update on weights to dropped-out units<br>
</li><li>Fixed bug in error calculation, was not dividing by minibatch size<br>
</li><li>Fixed bug in minibatch loading code for large datasets<br>
</li><li>Initial weight standard deviation is now proportional to 1/sqrt(VisibleUnits) so that the hidden activations will not blow up when presented with larger visible input vectors</li></ul>

<h3>4/14/2013 - <a href='http://visual-rbm.googlecode.com/svn-history/r162/trunk/release/VisualRBM.zip'>VisualRBM Build r162</a>, <a href='http://visual-rbm.googlecode.com/svn-history/r162/trunk/release/Tools.zip'>Tools Build r162</a></h3>
<ul><li>Improved performance by about 2-3x (depending on network architecture) by removing the periodic GPU->CPU weight transfer for use with the dynamic learning rate  scaling (which is no longer performed)<br>
<ul><li>On average the weight scales calculated did not vary very much<br>
</li><li>Learning rates will need to be tweaked a bit from values in previous VisualRBM versions<br>
</li></ul></li><li>Integrated the <a href='http://code.google.com/p/sickl/'>Simple Compute Kernel Library</a> which encapsulates all GPU initialization and shader nonsense, so development of new features will be much easier in the future<br>
</li><li>Speed improvement summary for MNIST over 100 epochs (clrbm in quiet mode on a GeForce GTX 660 Ti):<br>
<table><thead><th> </th><th> 100 Hidden </th><th> 500 Hidden </th><th> 2000 Hidden </th><th> 4000 Hidden </th></thead><tbody>
<tr><td> Version 157 </td><td> 6m 38s </td><td> 10m 35s </td><td> 30m 7s </td><td> 1h 13s </td></tr>
<tr><td> Version 162 </td><td> 2m 1s </td><td> 4m 21s </td><td> 15m 26s </td><td> 29m 52s </td></tr>
<tr><td> Improvement </td><td> 3.3x </td><td> 2.4x </td><td> 2x </td><td> 2x </td></tr></li></ul></tbody></table>


<h3>1/31/2013 - <a href='http://visual-rbm.googlecode.com/svn-history/r157/trunk/release/VisualRBM.zip'>VisualRBM Build r157</a>, <a href='http://visual-rbm.googlecode.com/svn-history/r157/trunk/release/Tools.zip'>Tools Build r157</a></h3>
<ul><li>Fixed issue in IDX to GPU transfer method for large IDX files where last set of minibatches would contain the same minibatch repeatedly, resulting in overfitting</li></ul>

<h3>1/31/2013</h3>
<blockquote>There is a bug in the large-dataset related training code in VisualRBM and clrbm (see <a href='https://groups.google.com/d/msg/visual-rbm/tOo0e-RtUIA/BkeDCRripzYJ'>https://groups.google.com/d/msg/visual-rbm/tOo0e-RtUIA/BkeDCRripzYJ</a> for details).  A new published build should be up tonight that fixes this issue.  Small datasets that are less than 1 GB (training + validation combined) are unaffected.  Sorry for the inconvenience!</blockquote>

<h3>1/27/2013 - <a href='http://visual-rbm.googlecode.com/svn-history/r154/trunk/release/VisualRBM.zip'>VisualRBM Build r154</a>, <a href='http://visual-rbm.googlecode.com/svn-history/r154/trunk/release/Tools.zip'>Tools Build r154</a></h3>
<ul><li>Added support to various command-line utilities for IDX files greater than 4 GB<br>
</li><li>Added support to VisualRBM and clrbm for IDX files greater than 4 GB<br>
</li><li>VisualRBM will no longer offers to automatically normalize data when using Gaussian visible units<br>
</li><li>RBM file format no longer includes mean or standard deviation statistics about the data it was trained on (see FileFormats page for specs for the new format)</li></ul>

<h3>1/12/2013 - <a href='http://visual-rbm.googlecode.com/svn-history/r146/trunk/release/Tools.zip'>Tools Build r146</a></h3>
<ul><li>Added <b>csv2idx</b> and <b>idx2csv</b> command line programs to make data conversion easier!</li></ul>

<h3>12/14/2012 - <a href='http://visual-rbm.googlecode.com/svn-history/r138/trunk/release/VisualRBM.zip'>VisualRBM Build r138</a>, <a href='http://visual-rbm.googlecode.com/svn-history/r138/trunk/release/Tools.zip'>Tools Build r138</a></h3>
<ul><li>Shader source is now compiled directly into the executable, so NativeShaders folder is no longer required at runtime.</li></ul>

<h3>12/12/2012 - <a href='http://visual-rbm.googlecode.com/svn-history/r128/trunk/release/VisualRBM.zip'>VisualRBM Build r128</a>, <a href='http://visual-rbm.googlecode.com/svn-history/r131/trunk/release/Tools.zip'>Tools Build r131</a></h3>

<ul><li>Added <b>splitidx</b> and <b>calchidden</b> command-line programs to the Tools.zip package.<br>
</li><li><b>splitidx</b> allows the user to breakup a single IDX data file into multiple subsets (for instance, dividing one large dataset into a training and test set).<br>
</li><li><b>calchidden</b> allows the user to calculate the hidden probabilities or the hidden activations of an entire dataset given a trained RBM model.  The user can also specify the amount of visible dropout used so that the hidden unit activations don't get overloaded.<br>
</li><li>Parameter Load/Save/Reset buttons are once again enabled when trainer is 'Paused'<br>
</li><li>Fixed Epoch error calculation (was previously using the 250 iteration moving average used in the Graph tab).<br>
</li><li>Fixed strange issue where printed error would be inconsistent between training runs during same session.<br>
</li><li>Fixed  <a href='https://code.google.com/p/visual-rbm/issues/detail?id=4'>Issue 4</a>  where program would crash sometimes on Stop.</li></ul>

<h3>11/18/2012 - <a href='http://visual-rbm.googlecode.com/svn-history/r58/trunk/release/Tools.zip'>Tools Build r58</a></h3>
<ul><li>Added <b>joinidx</b>,<b>catidx</b> and <b>idxinfo</b> command-line programs to the Tools.zip package.<br>
</li><li><b>joinidx</b> allows multiple IDX files with the same number of rows with different row length to be joined together.  For example, you can 'join' a dataset with 10,000 length 100 rows and a dataset with 10,000 length 150 rows into a new dataset with 10,000 length 250 rows.<br>
</li><li><b>catidx</b> allows multiple IDX files with identical row dimensions to be concatenated together into a larger dataset.  For example, you can 'cat' a dataset with 10,000 length 100 rows and a dataset with 20,000 length 100 rows into a new dataset with 30,000 100 rows.<br>
</li><li><b>idxinfo</b> prints out the header information of an IDX file.</li></ul>

<h3>11/14/2012</h3>
<ul><li>Created a Google Groups forum for the project: <a href='https://groups.google.com/forum/#!forum/visual-rbm'>https://groups.google.com/forum/#!forum/visual-rbm</a></li></ul>

<h3>11/11/2012 - <a href='http://visual-rbm.googlecode.com/svn-history/r46/trunk/release/VisualRBM.zip'>VisualRBM Build r46</a>, <a href='http://visual-rbm.googlecode.com/svn-history/r46/trunk/release/Tools.zip'>Tools Build r46</a></h3>
<ul><li>When loading data for an RBM with Gaussian visible units, VisualRBM will now ask before normalizing the training data.<br>
</li><li>Fixed two OpenGL bugs reported and fixed by step9899 (<a href='https://code.google.com/p/visual-rbm/issues/detail?id=7'>Issue 7</a> and <a href='https://code.google.com/p/visual-rbm/issues/detail?id=8'>Issue 8</a>).  <b>VisualRBM should now run on AMD cards properly!</b>.<br>
</li><li>Fixed intermediates location in VisualRBM solution file.<br>
</li><li>Split off clrbm to a seperate Tools solution so VisualRBM.sln will be less cluttered.  (needs to be run in the same directory containing the 'NativeShaders' directory packaged with VisualRBM; this will be consolidated later).</li></ul>

<h3>10/14/2012 - <a href='http://visual-rbm.googlecode.com/svn-history/r38/trunk/release/VisualRBM.zip'>VisualRBM Build r38</a></h3>

<ul><li>Added ability to load a second dataset during while paused rather than having to export RBM, stop, import RBM and reload data<br>
</li><li>Updated UI to include dropout parameters<br>
</li><li>Parameters files can now be loaded when training is paused, but only training parameters will be updated, not model parameters.  When training is stopped, all parameters will be updated.<br>
</li><li>Changed default L2 regularization to 0.<br>
</li><li>Fixed gaussian visible unit initialization bug;  Was initializing visibles to the mean value when they should have been initialized with 0.0 (since the training data is normalized when using gaussian visible units).  Gaussian visible unit models should now work better.<br>
</li><li>Fixed some internal OpenGL errors to be compliant with 3.3<br>
</li><li>Fixed reconstruction error calculation to take Visible Dropout into account<br>
</li><li>Fixed bug in minibatch shuffling logic<br>
</li><li>Fixed bug in vrbmparameter file parsing in VisualRBM</li></ul>

<h3>10/8/2012 - <a href='http://visual-rbm.googlecode.com/svn-history/r34/trunk/release/VisualRBM.zip'>VisualRBM Build r34</a></h3>

<ul><li>Added command line version, clrbm.exe (uses same parameters file as VisualRBM)<br>
</li><li>Fixed bug where we would report insufficient OpenGL version available<br>
</li><li>Added dropout support for visible units<br>
</li><li>Added support for configuring dropout values in VisualRBM via the vrbmparameters file (ability to modify in the GUI will come with the next release)</li></ul>

<h3>9/10/2012 - <a href='http://visual-rbm.googlecode.com/svn-history/r26/trunk/release/VisualRBM.zip'>VisualRBM Build r29</a></h3>

<ul><li>Changed initial value of Hidden Unit count to 100 (had been accidentally changed to 0)<br>
</li><li>Implemented dropout regularization (as described in Geoffrey Hinton's latest talk).  Currently dropout rate is set to 0.5, so at any given time only half of the hidden units will be active.  Cannot be changed in the UI yet and not saved off with parameters.  During testing, visible activations should be halved when using all units so they get the same expected value.</li></ul>

<h3>9/9/2012 - <a href='http://visual-rbm.googlecode.com/svn-history/r26/trunk/release/VisualRBM.zip'>VisualRBM Build r26</a></h3>

<ul><li>Now checks to see GPU supports OpenGL 3.3 and will tell user so<br>
</li><li>OpenGL context is now properly destroyed on program exit<br>
</li><li>Tooltips are a bit better<br>
</li><li>Bug Fixes</li></ul>

<h3>9/8/2012 - <a href='http://visual-rbm.googlecode.com/svn-history/r20/trunk/release/VisualRBM.zip'>VisualRBM Build r20</a></h3>

<ul><li>Reduced required OpenGL version to 3.3 which should cover more users<br>
</li><li>Bug Fixes</li></ul>

<h3>9/7/2012</h3>

<ul><li>Added C# helper classes for IDX and RBM filetypes