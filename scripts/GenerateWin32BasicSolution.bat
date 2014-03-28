call scripts\cleancmake.bat basic
cd samples\basic
cmake -G"Visual Studio 11" -Tv110_xp -DLibLogicalAccess_DIR=./../../install/lib/cmake/liblogicalaccess -DCMAKE_INSTALL_PREFIX=../../install .
cd ..\..
pause;