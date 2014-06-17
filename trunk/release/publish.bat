:: directories
set BIN="..\bin\Release"

:: solutions
set OMLT="..\source\OMLT\OMLT.sln"
set VISUALRBM="..\source\VisualRBM\VisualRBM.sln"
set TOOLS="..\source\Tools\Tools.sln"

:: executables
set ZIP="C:\Program Files\7-Zip\7z.exe"
set DEVENV="C:\Program Files (x86)\Microsoft Visual Studio 10.0\Common7\IDE\devenv.exe"

:: delete odl releases

del VisualRBM.zip
del Tools.zip

:: clean build

%DEVENV% %OMLT% /Clean Release
%DEVENV% %VISUALRBM% /Clean Release
%DEVENV% %TOOLS% /Clean Release

:: build releases

%DEVENV% %OMLT% /Build Release
%DEVENV% %VISUALRBM% /Build Release
%DEVENV% %TOOLS% /Build Release

:: create zips

%ZIP% a VisualRBM.zip %BIN%\VisualRBM.exe %BIN%\VisualRBMInterop.dll
%ZIP% a Tools.zip %BIN%\cltrain.exe %BIN%\catidx.exe %BIN%\joinidx.exe %BIN%\idxinfo.exe %BIN%\splitidx.exe %BIN%\calchidden.exe %BIN%\idx2csv.exe %BIN%\csv2idx.exe %BIN%\image2csv.exe %BIN%\shuffleidx.exe %BIN%\buildmlp.exe

pause
