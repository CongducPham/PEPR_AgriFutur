@ECHO OFF
SET back=%cd%

c++ src/xlpp.cpp -o xlpp.o -c

FOR /d %%i IN (test\*) DO (
cd "%%i"
cd

c++ -o main.exe main.cpp ../../xlpp.o
.\main.exe
if errorlevel 1 (
   echo Test failed.
   exit /b %errorlevel%
)
echo Test completed.

)
cd %back%