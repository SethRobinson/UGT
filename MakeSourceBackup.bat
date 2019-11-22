call app_info_setup.bat
set FNAME=%APP_NAME%_Source_%DATE:~4,2%_%DATE:~7,2%.zip
cd ..
del %FNAME%
:-x!*.svn -
dir
shared\win\utils\7za.exe a -r -tzip %FNAME% %APP_NAME%\* base_setup.bat -x!*.zip -x!*.csproj -x!*.sln -x!log.txt -x!*.ncb -x!*.bsc -x!*.pdb -x!*.pch -x!*.sbr -x!*.ilk -x!*.idb -x!.o -x!*.obj -x!*.DS_Store -x!._* -x!%APP_NAME%\dist -x!%APP_NAME%\build\web -x!%APP_NAME%\build\win -x!%APP_NAME%\.vs -x!%APP_NAME%\dist -x!%APP_NAME%\Library -x!%APP_NAME%\Temp -x!%APP_NAME%\webgltemp
cd %APP_NAME%
pause