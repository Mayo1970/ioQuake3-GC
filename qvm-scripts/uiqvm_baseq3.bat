@echo off
cd ..
mkdir qvm 2>nul
mkdir qvm\build 2>nul
mkdir qvm\baseq3 2>nul
mkdir qvm\baseq3\vm 2>nul
mkdir qvm\build\q3_ui 2>nul
copy qvm-scripts\binaries\q3lcc.exe  qvm\build\q3_ui\ >nul
copy qvm-scripts\binaries\q3cpp.exe  qvm\build\q3_ui\ >nul
copy qvm-scripts\binaries\q3rcc.exe  qvm\build\q3_ui\ >nul
copy qvm-scripts\binaries\q3asm.exe  qvm\build\q3_ui\ >nul
copy qvm-scripts\asm-files\uiqvm_baseq3.q3asm qvm\build\ >nul

set LIBRARY=
set INCLUDE=

set cc=q3lcc -DQ3_VM -S -Wf-target=bytecode -I..\..\..\code\q3_ui -I..\..\..\code\qcommon %1

cd qvm\build\q3_ui

%cc%  ../../../code/q3_ui/ui_addbots.c
%cc%  ../../../code/q3_ui/ui_atoms.c
%cc%  ../../../code/q3_ui/ui_cdkey.c
%cc%  ../../../code/q3_ui/ui_cinematics.c
%cc%  ../../../code/q3_ui/ui_confirm.c
%cc%  ../../../code/q3_ui/ui_connect.c
%cc%  ../../../code/q3_ui/ui_controls2.c
%cc%  ../../../code/q3_ui/ui_credits.c
%cc%  ../../../code/q3_ui/ui_demo2.c
%cc%  ../../../code/q3_ui/ui_display.c
%cc%  ../../../code/q3_ui/ui_gameinfo.c
%cc%  ../../../code/q3_ui/ui_ingame.c
%cc%  ../../../code/q3_ui/ui_loadconfig.c
%cc%  ../../../code/q3_ui/ui_main.c
%cc%  ../../../code/q3_ui/ui_menu.c
%cc%  ../../../code/q3_ui/ui_mfield.c
%cc%  ../../../code/q3_ui/ui_mods.c
%cc%  ../../../code/q3_ui/ui_network.c
%cc%  ../../../code/q3_ui/ui_options.c
%cc%  ../../../code/q3_ui/ui_playermodel.c
%cc%  ../../../code/q3_ui/ui_players.c
%cc%  ../../../code/q3_ui/ui_playersettings.c
%cc%  ../../../code/q3_ui/ui_preferences.c
%cc%  ../../../code/q3_ui/ui_qmenu.c
%cc%  ../../../code/q3_ui/ui_removebots.c
%cc%  ../../../code/q3_ui/ui_saveconfig.c
%cc%  ../../../code/q3_ui/ui_serverinfo.c
%cc%  ../../../code/q3_ui/ui_servers2.c
%cc%  ../../../code/q3_ui/ui_setup.c
%cc%  ../../../code/q3_ui/ui_sound.c
%cc%  ../../../code/q3_ui/ui_sparena.c
%cc%  ../../../code/q3_ui/ui_specifyserver.c
%cc%  ../../../code/q3_ui/ui_splevel.c
%cc%  ../../../code/q3_ui/ui_sppostgame.c
%cc%  ../../../code/q3_ui/ui_spskill.c
%cc%  ../../../code/q3_ui/ui_startserver.c
%cc%  ../../../code/q3_ui/ui_team.c
%cc%  ../../../code/q3_ui/ui_teamorders.c
%cc%  ../../../code/q3_ui/ui_video.c

copy  ..\..\..\code\ui\ui_syscalls.asm ui_syscalls.asm >nul

%cc%  ../../../code/game/bg_lib.c
%cc%  ../../../code/game/bg_misc.c

%cc%  ../../../code/qcommon/q_math.c
%cc%  ../../../code/qcommon/q_shared.c

q3asm -f ../uiqvm_baseq3

cd ..\..
rmdir /S /Q build
cd ..\
cd qvm-scripts
