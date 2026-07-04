@echo off
cd ..
mkdir qvm 2>nul
mkdir qvm\build 2>nul
mkdir qvm\missionpack 2>nul
mkdir qvm\missionpack\vm 2>nul
mkdir qvm\build\cgame 2>nul
copy qvm-scripts\binaries\q3lcc.exe  qvm\build\cgame\ >nul
copy qvm-scripts\binaries\q3cpp.exe  qvm\build\cgame\ >nul
copy qvm-scripts\binaries\q3rcc.exe  qvm\build\cgame\ >nul
copy qvm-scripts\binaries\q3asm.exe  qvm\build\cgame\ >nul
copy qvm-scripts\asm-files\cgameqvm_missionpack.q3asm qvm\build\cgame.q3asm >nul

set LIBRARY=
set INCLUDE=

set cc=q3lcc -DMISSIONPACK -DSCRIPTHUD -DQ3_VM -S -Wf-target=bytecode -I..\..\..\code\cgame -I..\..\..\code\ui -I..\..\..\code\qcommon %1

cd qvm\build\cgame

%cc%  ../../../code/cgame/cg_consolecmds.c
%cc%  ../../../code/cgame/cg_draw.c
%cc%  ../../../code/cgame/cg_drawtools.c
%cc%  ../../../code/cgame/cg_effects.c
%cc%  ../../../code/cgame/cg_ents.c
%cc%  ../../../code/cgame/cg_event.c
%cc%  ../../../code/cgame/cg_info.c
%cc%  ../../../code/cgame/cg_localents.c
%cc%  ../../../code/cgame/cg_main.c
%cc%  ../../../code/cgame/cg_marks.c
%cc%  ../../../code/cgame/cg_newdraw.c
%cc%  ../../../code/cgame/cg_particles.c
%cc%  ../../../code/cgame/cg_players.c
%cc%  ../../../code/cgame/cg_playerstate.c
%cc%  ../../../code/cgame/cg_predict.c
%cc%  ../../../code/cgame/cg_scoreboard.c
%cc%  ../../../code/cgame/cg_servercmds.c
%cc%  ../../../code/cgame/cg_snapshot.c
%cc%  ../../../code/cgame/cg_view.c
%cc%  ../../../code/cgame/cg_weapons.c

%cc%  ../../../code/game/bg_lib.c
%cc%  ../../../code/game/bg_misc.c
%cc%  ../../../code/game/bg_pmove.c
%cc%  ../../../code/game/bg_slidemove.c

copy  ..\..\..\code\cgame\cg_syscalls.asm cg_syscalls.asm >nul

%cc%  ../../../code/qcommon/q_math.c
%cc%  ../../../code/qcommon/q_shared.c

%cc%  ../../../code/ui/ui_shared.c

q3asm -f ../cgame

cd ..\..
rmdir /S /Q build
cd ..\
cd qvm-scripts
