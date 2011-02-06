call "C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\vcvarsall.bat"
devenv "..\nppludi.sln" /rebuild "Setup|Win32"

set version=
for /f "tokens=*" %%a in ('git rev-parse --short  HEAD') do (set version=%%a) 

echo !define VERSION %version% > nppludi.nsi
more nppludi.nsi.template >> nppludi.nsi

pause