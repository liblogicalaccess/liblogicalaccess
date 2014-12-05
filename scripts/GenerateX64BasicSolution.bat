call scripts\cleancmake.bat basic
cd samples\basic
cmake.exe -G"Visual Studio 12 Win64" -Tv120_xp -DLibLogicalAccess_DIR=./../../install/lib/x64/Debug/cmake/liblogicalaccess -DCMAKE_INSTALL_PREFIX=../../install .
cd ..\..