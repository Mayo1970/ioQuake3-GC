@echo off
cd ..\ioQuake3-PS3
mkdir qvm 2>nul
mkdir qvm\build 2>nul
mkdir qvm\missionpack 2>nul
mkdir qvm\missionpack\vm 2>nul
mkdir qvm\build\ui 2>nul
copy ..\qvm-scripts\binaries\q3lcc.exe  qvm\build\ui\ >nul
copy ..\qvm-scripts\binaries\q3cpp.exe  qvm\build\ui\ >nul
copy ..\qvm-scripts\binaries\q3rcc.exe  qvm\build\ui\ >nul
copy ..\qvm-scripts\binaries\q3asm.exe  qvm\build\ui\ >nul
copy ..\qvm-scripts\asm-files\uiqvm_missionpack.q3asm qvm\build\ >nul

set LIBRARY=
set INCLUDE=

set cc=q3lcc -DMISSIONPACK -DSCRIPTHUD  -DQ3_VM -S -Wf-target=bytecode -I..\..\..\code\ui -I..\..\..\code\qcommon %1

cd qvm\build\ui

%cc%  ../../../code/ui/ui_atoms.c
%cc%  ../../../code/ui/ui_gameinfo.c
%cc%  ../../../code/ui/ui_main.c
%cc%  ../../../code/ui/ui_players.c
%cc%  ../../../code/ui/ui_shared.c

copy  ..\..\..\code\ui\ui_syscalls.asm ui_syscalls.asm >nul

%cc%  ../../../code/game/bg_lib.c
%cc%  ../../../code/game/bg_misc.c

%cc%  ../../../code/qcommon/q_math.c
%cc%  ../../../code/qcommon/q_shared.c

q3asm -f ../uiqvm_missionpack

cd ..\..
rmdir /S /Q build
cd ..\..\
cd qvm-scripts
