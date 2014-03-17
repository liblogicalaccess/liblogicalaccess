cd ..
call scripts/cleancmake.bat
cmake -DCMAKE_BUILD_TYPE=Release -DTARGET_ARCH=Win32 -G"Visual Studio 11" -Tv110_xp .
pause;