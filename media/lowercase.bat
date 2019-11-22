REM  From http://windowsitpro.com/article/articleid/81612/jsi-tip-8379-how-do-i-convert-a-file-name-to-lowercase.html

@echo off
if {%1}=={} @echo Syntax: LwrCase FullyQualifiedFileName&goto :EOF
if not exist %1 @echo LwrCase - %1 NOT found.&goto :EOF
setlocal
for /f "Tokens=*" %%a in ('@echo %~a1') do (
 set file=%%a
)
if /i "%file:~0,1%" EQU "d" @echo LwrCase - %1 is NOT a file.&endlocal&goto :EOF
for /f "Tokens=*" %%f in ('dir %1 /L /b /a /a-d') do (
 Rename %1 "%%f"
)
endlocal
