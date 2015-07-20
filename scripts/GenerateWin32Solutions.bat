call scripts\cleancmake.bat
cmake.exe -DCMAKE_BUILD_TYPE=Release -DTARGET_ARCH=Win32 -DLIB_SUFFIX=Win32/Release -G"Visual Studio 12" -Tv120_xp . -DCMAKE_INSTALL_PREFIX=./install .