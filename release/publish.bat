set BIN="E:\pospeselr\Projects\VisualRBM\bin\Release"
set ZIP="C:\Program Files\7-Zip\7z.exe"


del VisualRBM.zip
del Tools.zip

%ZIP% a VisualRBM.zip %BIN%\VisualRBM.exe %BIN%\QuickBoltzmannInterop.dll %BIN%\NativeShaders\
%ZIP% a Tools.zip %BIN%\clrbm.exe

pause
