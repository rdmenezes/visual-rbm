:: directories
set BIN="..\bin\Release"

:: solutions
set OMLT="..\source\OMLT\OMLT.sln"
set VISUALRBM="..\source\VisualRBM\VisualRBM.sln"
set TOOLS="..\source\Tools\Tools.sln"

:: executables
set DEVENV="C:\Program Files (x86)\Microsoft Visual Studio 10.0\Common7\IDE\devenv.exe"
set MAKENSIS="C:\Program Files (x86)\NSIS\makensis.exe"

:: delete odl releases

del VisualRBM.zip
del Tools.zip

:: clean build

:: %DEVENV% %OMLT% /Clean Release
:: %DEVENV% %VISUALRBM% /Clean Release
:: %DEVENV% %TOOLS% /Clean Release

:: build releases

:: %DEVENV% %OMLT% /Build Release
:: %DEVENV% %VISUALRBM% /Build Release
:: %DEVENV% %TOOLS% /Build Release

:: delete binaries from this folder
DEL *.exe
DEL *.dll

:: copy binaries to this folder
XCOPY %BIN%\VisualRBM.exe .
XCOPY %BIN%\VisualRBMInterop.dll .
XCOPY %BIN%\buildmlp.exe .
XCOPY %BIN%\calchidden.exe .
XCOPY %BIN%\catidx.exe .
XCOPY %BIN%\cltrain.exe .
XCOPY %BIN%\csv2idx.exe .
XCOPY %BIN%\idx2csv.exe .
XCOPY %BIN%\idxinfo.exe .
XCOPY %BIN%\image2csv.exe .
XCOPY %BIN%\joinidx.exe .
XCOPY %BIN%\shuffleidx.exe .
XCOPY %BIN%\splitidx.exe .

:: create installer

%MAKENSIS% installer.nsi

pause
