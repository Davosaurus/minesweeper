@echo OFF
g++ ms.cpp -o ms.exe -static-libstdc++ -static-libgcc
if errorlevel 1 (
   echo Compilation failed: error code #%errorlevel%
   cmd /k
)