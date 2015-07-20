call scripts\cleancmake.bat
cmake.exe -DCMAKE_BUILD_TYPE=Debug -DTARGET_ARCH=Win32 -DLIB_SUFFIX=Win32/Debug -G"Visual Studio 12" -Tv120_xp . -DCMAKE_INSTALL_PREFIX=./install .