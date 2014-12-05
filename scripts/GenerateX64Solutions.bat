call scripts\cleancmake.bat
cmake.exe -DCMAKE_BUILD_TYPE=Release -DTARGET_ARCH=x64 -DLIB_SUFFIX=x64/Release -G"Visual Studio 12 Win64" -Tv120_xp . -DCMAKE_INSTALL_PREFIX=./install .