@echo off
cd ..
mkdir qvm 2>nul
mkdir qvm\build 2>nul
mkdir qvm\missionpack 2>nul
mkdir qvm\missionpack\vm 2>nul
mkdir qvm\build\game 2>nul
copy qvm-scripts\binaries\q3lcc.exe  qvm\build\game\ >nul
copy qvm-scripts\binaries\q3cpp.exe  qvm\build\game\ >nul
copy qvm-scripts\binaries\q3rcc.exe  qvm\build\game\ >nul
copy qvm-scripts\binaries\q3asm.exe  qvm\build\game\ >nul
copy qvm-scripts\asm-files\qagameqvm_missionpack.q3asm qvm\build\game.q3asm >nul

set LIBRARY=
set INCLUDE=

set cc=q3lcc -DQ3_VM -DMISSIONPACK -S -Wf-target=bytecode -I..\..\..\code\game -I..\..\..\code\qcommon %1

cd qvm\build\game

%cc%  ../../../code/game/ai_chat.c
%cc%  ../../../code/game/ai_cmd.c
%cc%  ../../../code/game/ai_dmnet.c
%cc%  ../../../code/game/ai_dmq3.c
%cc%  ../../../code/game/ai_main.c
%cc%  ../../../code/game/ai_team.c
%cc%  ../../../code/game/ai_vcmd.c
%cc%  ../../../code/game/bg_lib.c
%cc%  ../../../code/game/bg_misc.c
%cc%  ../../../code/game/bg_pmove.c
%cc%  ../../../code/game/bg_slidemove.c
%cc%  ../../../code/game/g_active.c
%cc%  ../../../code/game/g_arenas.c
%cc%  ../../../code/game/g_bot.c
%cc%  ../../../code/game/g_client.c
%cc%  ../../../code/game/g_cmds.c
%cc%  ../../../code/game/g_combat.c
%cc%  ../../../code/game/g_items.c
%cc%  ../../../code/game/g_main.c
%cc%  ../../../code/game/g_mem.c
%cc%  ../../../code/game/g_misc.c
%cc%  ../../../code/game/g_missile.c
%cc%  ../../../code/game/g_mover.c
%cc%  ../../../code/game/g_session.c
%cc%  ../../../code/game/g_spawn.c
%cc%  ../../../code/game/g_svcmds.c
%cc%  ../../../code/game/g_target.c
%cc%  ../../../code/game/g_team.c
%cc%  ../../../code/game/g_trigger.c
%cc%  ../../../code/game/g_utils.c
%cc%  ../../../code/game/g_weapon.c

copy  ..\..\..\code\game\g_syscalls.asm g_syscalls.asm >nul

%cc%  ../../../code/qcommon/q_math.c
%cc%  ../../../code/qcommon/q_shared.c

q3asm -f ../game

cd ..\..
rmdir /S /Q build
cd ..\
cd qvm-scripts
