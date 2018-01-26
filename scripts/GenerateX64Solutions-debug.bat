call scripts\cleancmake.bat
cmake.exe -DCMAKE_BUILD_TYPE=Debug -DTARGET_ARCH=x64 -DLIB_SUFFIX=x64/Debug -G"Visual Studio 15 Win64" -Tv141_xp -DCMAKE_INSTALL_PREFIX=./install .