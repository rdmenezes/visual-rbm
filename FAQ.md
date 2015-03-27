# FAQ #

### What are the system requirements for VisualRBM? ###

VisualRBM requires a system with a GPU which supports OpenGL 3.3 or newer.  For Nvidia, this means a card from the GeForce 8 series or newer.  For ATI/AMD, thish means a Radeon HD series or newer.

### What do I do if I get the error message "VisualRBM requires a GPU that supports at least OpenGL 3.3" when running VisualRBM? ###

If VisualRBM claims your OpenGL version is too old but your GPU is newer than the aforementioned models, try updating your graphics drivers.

### VisualRBM crashes immediately on first run, how do I fix this? ###

This crash occurs because VisualRBM fails to allocate space for the data atlas (basically, a large block of allocated texture memory) which is used to store training and validation data during training.  You can specify a smaller data atlas by adding the -atlasSize=NUM flag, where **NUM** is an integer greater than 128.  This will specify the amount of memory to allocate for the data atlas in megabytes.  By default, VisualRBM allocates 512 megabytes of texture memory for the data atlas.

Try and find as large a value as possible for **NUM** which allows VisualRBM to run without crashing, since the more data that can be stored in memory on the GPU the less often VisualRBM will need to swap to disk.  Once you've found a good value for **NUM**, you should update the 'Target' field in the VisualRBM shortcut (on your Desktop and in your Start menu) to:

"C:\Program Files (x86)\VisualRBM\VisualRBM.exe" -atlasSize=NUM

### I have training data in XYZ format, how can I use it to train models with VisualRBM? ###

VisualRBM uses a slightly modified IDX file format (as described on the FileFormats page).  This is a binary format that allows VisualRBM to stream data from disk during training relatively quickly, rather than having to repeatedly parse human readable plain-text file formats.

If your data is in CSV format, it can be converted using the [csv2idx](https://code.google.com/p/visual-rbm/wiki/Tools#csv2idx) command-line program which comes with VisualRBM.  Various other programs for manipulating IDX files come installed with VisualRBM.  More info can be found on the [Tools](Tools.md) page.

Alternatively, I've also written a few classes in various languages for reading and writing IDX files:

  * C# - [IDX.cs](https://visual-rbm.googlecode.com/svn/trunk/helpers/IDX.cs)
  * C++ - [IDX.hpp ](https://visual-rbm.googlecode.com/svn/trunk/source/OMLT/OMLT/include/IDX.hpp)
  * Python - [IDX.py](https://visual-rbm.googlecode.com/svn/trunk/helpers/IDX.py)

### I have many gigabytes of training data that can't possibly all be stored in GPU RAM, what do I have to do to use VisualRBM? ###

You don't have to do anything at all!  VisualRBM will automatically stream data from disk if it cannot all be uploaded to the GPU.

To decrease the amount of time spent streaming from disk, you can create a virtual RAM disk using the [ImDisk Virtual Disk Driver](http://www.ltr-data.se/opencode.html/#ImDisk) and store your data there during training.  This way, you'll be streaming data from system RAM to GPU RAM instead from disk to GPU RAM.