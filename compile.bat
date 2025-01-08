@echo OFF
g++ ms.cpp -o ms.exe -static-libstdc++ -static-libgcc -Wl,--stack,1073741824
if errorlevel 1 (
   echo Compilation failed: error code #%errorlevel%
   cmd /k
)