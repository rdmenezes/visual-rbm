## Overview ##

The visual-rbm repository contains three Visual Studio 2010 solutions:
  * OMLT - The machine learning backend
  * VisualRBM - UI frontend and CLI/Interop code interfacing with OMLT
  * Tools - Command line tools for handling IDX data files as well as the command-line trainer

VisualRBM and associated tools have a little bit of a dependency tree:

  * OMLT (native static lib)
    * [cJSON](http://sourceforge.net/projects/cjson/)  (C source, slightly modified)
    * [SiCKL](https://code.google.com/p/sickl/) (native static lib)
      * [FreeGLUT](http://freeglut.sourceforge.net/) (native static lib)
      * [GLEW](http://glew.sourceforge.net/) (native static lib)

  * VisualRBM (C# Exe)
    * VisualRBMInterop (CLI Interop DLL)
      * OMLT

  * Tools
    * OMLT
    * stb\_image (header only C library)

The tools solution depends on OMLT and everything below it.

If you are interested in writing your own training tool or parsing and using trained models, you will need to create a new project which links against OMLT, SiCKL, FreeGLUT, and GLEW and which includes the headers for OMLT and SiCKL.

Libs, includes and source (for cJSON) can be found in the /trunk/extern directory.  A build script can be found in /trunk/release/publish.bat.  This script will do a full Release build for each of the solutions in order and stick the final executables in an intaller using NSIS.