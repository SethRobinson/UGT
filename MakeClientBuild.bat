call app_info_setup.bat

rmdir tempbuild /S /Q

mkdir tempbuild
mkdir tempbuild\audio
xcopy bin\audio tempbuild\audio /E /F /Y
mkdir tempbuild\interface
xcopy bin\interface tempbuild\interface /E /F /Y
xcopy bin\*.dll tempbuild
xcopy bin\curl-ca-bundle.crt tempbuild
xcopy bin\font_license.txt tempbuild
xcopy bin\config_template.txt tempbuild
xcopy bin\readme.txt tempbuild
ECHO F|xcopy "bin\UGT_Release GL_x64.exe" tempbuild\ugt.exe
xcopy bin\SourceHanSerif-Medium.ttc tempbuild


:Do the signing, not required, uses RTsoft's signing key with obviously isn't included on the GIT
copy tempbuild\ugt.exe ..\Signing
cd ..\Signing
call sign.bat ugt.exe "Universal Game Translator" "www.rtsoft.com"
cd ..\UGT
copy ..\Signing\ugt.exe tempbuild\ugt.exe

:create the archive
set FNAME=UniversalGameTranslator_Windows.zip
del %FNAME%
..\shared\win\utils\7za.exe a -r -tzip %FNAME% tempbuild

:Rename the root folder
..\shared\win\utils\7z.exe rn %FNAME% tempbuild\ UGT\

:Copy to Seth's machines he uses this on
call CopyToYogini.bat
call CopyToTennis.bat
pause