make clean
rm -fr Savestate2snes.app
export PATH="/Users/piko/Qt/5.12.2/clang_64/bin/:$PATH"
qmake
make
rm Savestate2snes.dmg
macdeployqt Savestate2snes.app -dmg

