@echo off
echo Compiling the qvm files - please wait.

REM Call the batch scripts
call cgameqvm_baseq3.bat
echo 1 of 6 Done
call cgameqvm_missionpack.bat
echo 2 of 6 Done
call qagameqvm_baseq3.bat
echo 3 of 6 Done
call qagameqvm_missionpack.bat
echo 4 of 6 Done
call uiqvm_baseq3.bat
echo 5 of 6 Done
call uiqvm_missionpack.bat
echo 6 or 6 Done
(
echo.
echo Check the qvm folder for the newly created files...
)