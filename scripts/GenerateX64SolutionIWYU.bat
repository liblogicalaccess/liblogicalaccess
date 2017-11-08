call scripts\cleancmake.bat
cmake.exe  -G Ninja -DCMAKE_BUILD_TYPE=Debug -DTARGET_ARCH=x64 -DLIB_SUFFIX=x64/Debug -DRUN_IWYU=True -DCMAKE_INSTALL_PREFIX=./install .
