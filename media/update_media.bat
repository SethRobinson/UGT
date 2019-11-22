echo You don't have to make media for this example, otherwise it will delete your bin/interface directory, but I want this on svn so it's easy to build this most simple example, even for
echo people who don't have windows to make the font.


REM Make fonts

set PACK_EXE=..\..\.\shared\win\utils\RTPack.exe

REM Delete all existing packed textures from this dir
cd interface
for /r %%f in (*.rttex) do del %%f
cd ..

for /r %%f in (font*.txt) do %PACK_EXE% -make_font %%f

REM Process our images and textures and copy them into the bin directory

REM -pvrtc4 for compressed, -pvrt4444 or -pvrt8888 (32 bit)  for uncompressed

:cd game
:for /r %%f in (*.bmp *.png) do ..\%PACK_EXE%  -pvrt8888 %%f
:cd ..

cd interface
for /r %%f in (*.bmp *.png) do ..\%PACK_EXE%  -pvrt8888 %%f
cd ..

REM Custom things that don't need preprocessing

REM Final compression
for /r %%f in (*.rttex) do %PACK_EXE% %%f

REM Delete things we don't want copied
del interface\font_*.rttex

:Commenting this out, just in case people build media anyway, don't want to erase a svn controlled dir
//rmdir ..\bin\interface /S /Q

REM copy the stuff we care about

mkdir ..\bin\interface
xcopy interface ..\bin\interface /E /F /Y /EXCLUDE:exclude.txt

:Special case, delete the .rttex, for this one example, we only want a .bmp there
del ..\bin\interface\test.rttex

REM Convert everything to lowercase, otherwise the iphone will choke on the files
REM for /r %%f in (*.*) do ..\media\LowerCase.bat  %%f

del icon.rttex
del default.rttex
pause
