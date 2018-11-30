call scripts\cleancmake.bat
cmake.exe -DCMAKE_BUILD_TYPE=Release -DTARGET_ARCH=x64 -DLIB_SUFFIX=x64/Release -G"Visual Studio 15 Win64" -Tv141 -DCMAKE_INSTALL_PREFIX=./install .