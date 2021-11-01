call app_info_setup.bat

rmdir tempbuild /S /Q

mkdir tempbuild
mkdir tempbuild\audio
xcopy bin\audio tempbuild\audio /E /F /Y
mkdir tempbuild\interface
xcopy bin\interface tempbuild\interface /E /F /Y
xcopy bin\*.dll tempbuild
xcopy bin\curl-ca-bundle.crt tempbuild
xcopy bin\siddhanta_font_license.txt tempbuild
xcopy bin\SourceHanSerif-Medium_font_license.txt tempbuild
xcopy bin\config_template.txt tempbuild
xcopy bin\fonts.txt tempbuild
xcopy bin\readme.txt tempbuild
ECHO F|xcopy "bin\UGT_FMOD_Release_GL_x64.exe" tempbuild\ugt.exe
xcopy bin\SourceHanSerif-Medium.ttc tempbuild
xcopy bin\siddhanta.ttf tempbuild
xcopy bin\lohit.punjabi.1.1.ttf tempbuild
xcopy bin\lohit_punjabi_1_1_ttf_license.txt tempbuild
mkdir tempbuild\htmlexport
xcopy bin\htmlexport\*.txt tempbuild\htmlexport
xcopy bin\htmlexport\*.css tempbuild\htmlexport

:Do the signing, not required, uses RTsoft's signing key with obviously isn't included on the GIT

call %RT_PROJECTS%\Signing\sign.bat tempbuild\ugt.exe "Universal Game Translator"
call %RT_PROJECTS%\Signing\sign.bat tempbuild\escapi.dll "Universal Game Translator"

:create the archive
set FNAME=UniversalGameTranslator_Windows.zip
del %FNAME%
..\shared\win\utils\7za.exe a -r -tzip %FNAME% tempbuild

:Rename the root folder
..\shared\win\utils\7z.exe rn %FNAME% tempbuild\ UGT\

:Copy to Seth's machines he uses this on
:call CopyToYogini.bat
:call CopyToTennis.bat
call CopyToRetro.bat
pause