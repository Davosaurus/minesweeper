@echo OFF
g++ ms.cpp -Og -g -o ms.exe -static-libstdc++ -Wall -static-libgcc -Wl,--stack,1073741824 -D_GLIBCXX_DEBUG
if errorlevel 1 (
   echo Compilation failed: error code #%errorlevel%
   cmd /k
)