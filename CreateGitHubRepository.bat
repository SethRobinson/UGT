@echo off
call app_info_setup.bat
echo You have to have already created the account on Github with the name %APP_NAME%.  Target base repository set with the GITHUBNAME var in ../setup_bars.bat
pause
echo Warning: This will add ALL files in the tree of the folder %APP_NAME% and commit them to https://github.com/%GITHUBNAME%/%APP_NAME%.git, you should check nothing sensitive (passwords,etc) was added before you do the final push command.
pause
@echo on
git init
git add .
git commit -m "First commit"
git remote add origin https://github.com/%GITHUBNAME%/%APP_NAME%.git
git remote -v
echo All done.  Check that the committed files look ok, then do a final push.
pause

REM guess I'll do the final push manually for safety, so commented out
REM git push -u origin master