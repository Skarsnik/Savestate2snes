@ECHO ON
set projectPath=C:\Users\Skarsnik\Documents\GitHub\Savestate2snes
set compilePath=D:\Project\compile\savestate2snes
set deployPath=D:\Project\deploy\savestate2snes
set originalBinDir=%compilePath%
::set vscdll=D:\Visual Studio\VC\redist\x64\Microsoft.VC140.CRT\msvcp140.dll

rmdir /Q /S %deployPath%
mkdir %deployPath%
:: Compile

:: D:\Visual Studio\VC\vcvarsall.bat amd64
d:
mkdir %compilePath%
cd %compilePath%
cd
qmake %projectPath%\savestate2snes.pro -spec win32-g++ "CONFIG+=release"
mingw32-make

xcopy /y %originalBinDir%\release\savestate2snes.exe %deployPath%

::mkdir %deployPath%\qml\
::xcopy /e %projectPath%\qml %deployPath%\qml /r

echo "Deploy QT"
windeployqt.exe --no-translations --no-system-d3d-compiler --no-opengl --no-svg --no-webkit --no-webkit2 --release %deployPath%\savestate2snes.exe
xcopy /e %projectPath%\Patches %deployPath%\Patches /r

:: Clean up Qt extra stuff
rmdir /Q /S %deployPath%\imageformats
del %deployPath%\opengl32sw.dll
del %deployPath%\libEGL.dll
del %deployPath%\libGLESV2.dll