cd ..
call scripts/cleancmake.bat
cmake -DCMAKE_BUILD_TYPE=Release -DTARGET_ARCH=x64 -G"Visual Studio 11 Win64" -Tv110_xp .
cd plugins/pluginscards/
cmake -DCMAKE_BUILD_TYPE=Release -DTARGET_ARCH=x64 -G"Visual Studio 11 Win64" -Tv110_xp .
cd ../pluginsreaderproviders/
cmake -DCMAKE_BUILD_TYPE=Release -DTARGET_ARCH=x64 -G"Visual Studio 11 Win64" -Tv110_xp .
cd ../../scripts
pause;