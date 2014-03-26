cd ..
call scripts/cleancmake.bat
cmake -DCMAKE_BUILD_TYPE=Release -DTARGET_ARCH=x64 -G"Visual Studio 11 Win64" -Tv110_xp .
cd samples/basic
cmake -G"Visual Studio 11 Win64" -Tv110_xp -DLibLogicalAccess_DIR=./../../install/lib/cmake/liblogicalaccess .
cd ../..
pause;