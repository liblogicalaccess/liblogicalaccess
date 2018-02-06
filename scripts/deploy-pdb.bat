for /f %%a IN ('dir /b /s "%1\*.pdb"') do call gitlink.exe -a %%a
symstore.exe add /r /f "%1" /s "%2" /t LibLogicalAccess /v %3 /compress