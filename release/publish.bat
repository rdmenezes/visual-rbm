set BIN="..\bin\Release"
set ZIP="C:\Program Files\7-Zip\7z.exe"

del VisualRBM.zip
del Tools.zip

%ZIP% a VisualRBM.zip %BIN%\VisualRBM.exe %BIN%\QuickBoltzmannInterop.dll
%ZIP% a Tools.zip %BIN%\clrbm.exe %BIN%\catidx.exe %BIN%\joinidx.exe %BIN%\idxinfo.exe %BIN%\splitidx.exe %BIN%\calchidden.exe

pause
