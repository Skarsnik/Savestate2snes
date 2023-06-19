@ECHO ON
set projectPath=D:\Project\Savestate2snes
set compilePath=D:\Project\compile\savestate2snes
set deployPath=D:\Project\deploy\Savestate2snes
set originalBinDir=%compilePath%
set vscdll=C:\Visual Studio\VC\Redist\MSVC\14.12.25810\x64\Microsoft.VC141.CRT\msvcp140.dll

rmdir /Q /S %compilePath%
mkdir %compilePath%
rmdir /Q /S %deployPath%
mkdir %deployPath%
:: Compile

::"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" amd64

mkdir %compilePath%
cd %compilePath%
cd
set QMAKE_MSC_VER=1910
qmake %projectPath%\savestate2snes.pro  -spec win32-msvc "CONFIG+=release no_batch"
nmake

xcopy /y %originalBinDir%\release\savestate2snes.exe %deployPath%

echo "Deploy QT"
windeployqt.exe --no-translations --no-system-d3d-compiler --no-opengl --no-svg --no-webkit --no-webkit2 --release %deployPath%\savestate2snes.exe
xcopy /e %projectPath%\Patches %deployPath%\Patches\ /r

:: Clean up Qt extra stuff
rmdir /Q /S %deployPath%\imageformats
del %deployPath%\opengl32sw.dll
del %deployPath%\libEGL.dll
del %deployPath%\libGLESV2.dll
del %deployPath%\vc_redist.x64.exe

xcopy /y "%vscdll%" %deployPath%

echo Generating Translation

mkdir %deployPath%\i18n

lrelease %projectPath%\Savestate2snes.pro
xcopy /y %projectPath%\Translations\savestate2snes_fr.qm %deployPath%\i18n
xcopy /y %projectPath%\Translations\savestate2snes_de.qm %deployPath%\i18n
xcopy /y %projectPath%\Translations\savestate2snes_sv.qm %deployPath%\i18n
xcopy /y %projectPath%\Translations\savestate2snes_nl.qm %deployPath%\i18n

xcopy /y F:\Project\snesclassicstuff\serverstuff\hmod\serverstuff.hmod %deployPath%


xcopy /y %projectPath%\License-GPL3.txt %deployPath%
xcopy /i /y %projectPath%\README.md %deployPath%\Readme.txt
xcopy /y %projectPath%\icon64x64.png %deployPath%\icone.png


cd %projectPath%