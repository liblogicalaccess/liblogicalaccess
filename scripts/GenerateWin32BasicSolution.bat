call scripts\cleancmake.bat basic
cd samples\basic
cmake -G"Visual Studio 11" -Tv110_xp -DLibLogicalAccess_DIR=./../../install/lib/Win32/Debug/cmake/liblogicalaccess -DCMAKE_INSTALL_PREFIX=../../install .
cd ..\..