#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------
ifeq ($(strip $(DEVKITPPC)),)
$(error "Please set DEVKITPPC in your environment. export DEVKITPPC=<path to>devkitPPC")
endif

FLAVOR ?= q3a
MAKEFILE_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
include $(MAKEFILE_DIR)flavors.mk

include $(DEVKITPPC)/gamecube_rules

#---------------------------------------------------------------------------------
DEBUG ?= 0
ifeq ($(DEBUG),1)
DEBUG_CFLAGS	:=	-DGC_DEBUG -g
DEBUG_LDFLAGS	:=	-g
else
DEBUG_CFLAGS	:=
DEBUG_LDFLAGS	:=
endif

DISC_ID ?= GQ3E
ISO_ROOT ?=
GC_MEM1_MB ?=

QCOMMON_OBJ	:=	$(addprefix code/qcommon/,cmd.o cm_load.o cm_patch.o \
			cm_polylib.o cm_test.o cm_trace.o common.o cvar.o files.o \
			huffman.o ioapi.o md4.o md5.o msg.o net_chan.o q_math.o \
			q_shared.o unzip.o vm.o puff.o)

SERVER_OBJ	:=	$(addprefix code/server/,sv_bot.o sv_ccmds.o sv_client.o \
			sv_game.o sv_init.o sv_main.o sv_net_chan.o sv_snapshot.o \
			sv_world.o)

BOTLIB_OBJ	:=	$(addprefix code/botlib/,be_aas_bspq3.o be_aas_cluster.o \
			be_aas_debug.o be_aas_entity.o be_aas_file.o be_aas_main.o \
			be_aas_move.o be_aas_optimize.o be_aas_reach.o be_aas_route.o \
			be_aas_routealt.o be_aas_sample.o be_ai_char.o be_ai_chat.o \
			be_ai_gen.o be_ai_goal.o be_ai_move.o be_ai_weap.o \
			be_ai_weight.o be_ea.o be_interface.o l_crc.o l_libvar.o \
			l_log.o l_memory.o l_precomp.o l_script.o l_struct.o)

CLIENT_OBJ	:=	$(addprefix code/client/,cl_avi.o cl_cgame.o cl_cin.o \
			cl_console.o cl_input.o cl_keys.o cl_main.o cl_net_chan.o \
			cl_parse.o cl_scrn.o cl_ui.o snd_main.o snd_dma.o snd_mem.o \
			snd_mix.o snd_adpcm.o snd_wavelet.o snd_codec.o \
			snd_codec_wav.o)

SYS_OBJ		:=	$(addprefix code/sys/,gc_main.o gc_input.o sys_gc.o gc_dvd.o gc_aram.o \
			gc_glimp.o gc_null_snddma.o gc_net_null.o gc_modules.o)

RENDERER_OBJ	:=	$(addprefix code/renderer/,tr_animation.o \
			tr_backend.o tr_bsp.o tr_cmds.o tr_curve.o tr_flares.o \
			tr_font.o tr_image.o tr_image_png.o tr_image_jpg.o \
			tr_image_bmp.o tr_image_tga.o tr_image_pcx.o tr_init.o \
			tr_light.o tr_main.o tr_marks.o tr_mesh.o tr_model.o \
			tr_model_iqm.o tr_noise.o tr_scene.o tr_shade.o \
			tr_shade_calc.o tr_shader.o tr_shadows.o tr_sky.o \
			tr_surface.o tr_world.o tr_gx.o tr_gx_texture.o qgl_gc.o)

JPEG_OBJ	:=	$(addprefix code/jpeg-8c/,jaricom.o jcapimin.o \
			jcapistd.o jcarith.o jccoefct.o jccolor.o jcdctmgr.o \
			jchuff.o jcinit.o jcmainct.o jcmarker.o jcmaster.o \
			jcomapi.o jcparam.o jcprepct.o jcsample.o jctrans.o \
			jdapimin.o jdapistd.o jdarith.o jdatadst.o jdatasrc.o \
			jdcoefct.o jdcolor.o jddctmgr.o jdhuff.o jdinput.o \
			jdmainct.o jdmarker.o jdmaster.o jdmerge.o jdpostct.o \
			jdsample.o jdtrans.o jerror.o jfdctflt.o jfdctfst.o \
			jfdctint.o jidctflt.o jidctfst.o jidctint.o jmemmgr.o \
			jmemnobs.o jquant1.o jquant2.o jutils.o)

ZLIB_CFILES	:=	$(notdir $(wildcard $(CURDIR)/code/zlib/*.c))
ZLIB_OBJ	:=	$(addprefix code/zlib/,$(ZLIB_CFILES:.c=.o))

MODULE_DIR	:=	modules
MODULE_OBJS	:=	$(addprefix $(MODULE_DIR)/,$(addsuffix .o,qagame cgame ui))

MOD_SHARED	:=	code/qcommon/q_math.c code/qcommon/q_shared.c
MOD_BG		:=	code/game/bg_misc.c code/game/bg_pmove.c \
			code/game/bg_slidemove.c code/game/bg_lib.c

qagame_SRCS	:=	$(addprefix code/game/,g_main.c ai_chat.c ai_cmd.c \
			ai_dmnet.c ai_dmq3.c ai_main.c ai_team.c ai_vcmd.c g_active.c \
			g_arenas.c g_bot.c g_client.c g_cmds.c g_combat.c g_items.c \
			g_mem.c g_misc.c g_missile.c g_mover.c g_session.c g_spawn.c \
			g_svcmds.c g_target.c g_team.c g_trigger.c g_utils.c g_weapon.c \
			g_syscalls.c) $(MOD_BG) $(MOD_SHARED)

cgame_SRCS	:=	$(addprefix code/cgame/,cg_main.c cg_consolecmds.c cg_draw.c \
			cg_drawtools.c cg_effects.c cg_ents.c cg_event.c cg_info.c \
			cg_localents.c cg_marks.c cg_particles.c cg_players.c \
			cg_playerstate.c cg_predict.c cg_scoreboard.c cg_servercmds.c \
			cg_snapshot.c cg_view.c cg_weapons.c cg_syscalls.c) \
			$(MOD_BG) $(MOD_SHARED)

ui_SRCS		:=	$(addprefix code/q3_ui/,ui_main.c ui_addbots.c ui_atoms.c \
			ui_cdkey.c ui_cinematics.c ui_confirm.c ui_connect.c \
			ui_controls2.c ui_credits.c ui_demo2.c ui_display.c \
			ui_gameinfo.c ui_ingame.c ui_loadconfig.c ui_menu.c ui_mfield.c \
			ui_mods.c ui_network.c ui_options.c ui_playermodel.c \
			ui_players.c ui_playersettings.c ui_preferences.c ui_qmenu.c \
			ui_removebots.c ui_saveconfig.c ui_serverinfo.c ui_servers2.c \
			ui_setup.c ui_sound.c ui_sparena.c ui_specifyserver.c \
			ui_splevel.c ui_sppostgame.c ui_spskill.c ui_startserver.c \
			ui_team.c ui_teamorders.c ui_video.c) code/game/bg_misc.c \
			code/game/bg_lib.c $(addprefix code/ui/,ui_syscalls.c) \
			$(MOD_SHARED)

CFLAGS		=	-O2 -Wall $(MACHDEP) -G 0 -msdata=none -DHW_DOL \
			-DNO_VM_COMPILED -DUSE_LOCAL_HEADERS -DGC_DISC_ID=\"$(DISC_ID)\" \
			$(FLAVOR_CFLAGS) $(DEBUG_CFLAGS) $(INCLUDE)

ifneq ($(GC_MEM1_MB),)
CFLAGS		+=	-DGC_MEM1_MB=$(GC_MEM1_MB)
GC_MEM1_HI	:=	$(shell printf '0x%08x' $$((0x80000000 + $(GC_MEM1_MB) * 0x100000)))
LDFLAGS		=	$(MACHDEP) -G 0 -msdata=none -Wl,-Map,$(OUTPUT).map \
			-Wl,--defsym,__Arena1Hi=$(GC_MEM1_HI) $(DEBUG_LDFLAGS)
else
LDFLAGS		=	$(MACHDEP) -G 0 -msdata=none -Wl,-Map,$(OUTPUT).map $(DEBUG_LDFLAGS)
endif

LIBS		:=	-lfat -logc -lm

LIBDIRS		:=

#---------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))

export OUTPUT	:=	$(CURDIR)/$(BUILD)/$(TARGET)

export VPATH	:=	$(CURDIR)

export DEPSDIR	:=	$(CURDIR)/$(BUILD)

export LD	:=	$(CC)

export OFILES	:=	$(QCOMMON_OBJ) $(SERVER_OBJ) $(BOTLIB_OBJ) \
			$(CLIENT_OBJ) $(SYS_OBJ) $(ZLIB_OBJ) \
			$(RENDERER_OBJ) $(JPEG_OBJ) $(MODULE_OBJS)

export BOTLIB_OBJ
export ZLIB_OBJ
export JPEG_OBJ
export REPO_ROOT :=	$(CURDIR)

OBJDIRS		:=	$(sort $(dir $(OFILES)))

export INCLUDE	:=	$(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
			$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
			-I$(CURDIR)/$(BUILD) \
			-I$(LIBOGC_INC)

export LIBPATHS	:=	-L$(LIBOGC_LIB) $(foreach dir,$(LIBDIRS),-L$(dir)/lib)

.PHONY: all clean iso $(BUILD)

all: $(BUILD)

$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@for d in $(OBJDIRS); do mkdir -p $@/$$d; done
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

clean:
	@echo clean ...
	@rm -rf build build_* *.gcm *.iso
	@$(MAKE) --no-print-directory -C tools/apploader clean

iso: all
	@$(MAKE) -C tools/apploader
	@python3 tools/mkgcm.py --dol $(BUILD)/$(TARGET).dol --apploader build/apploader/apploader.bin --out $(BUILD)/$(TARGET).gcm --id $(DISC_ID) --maker QG --name "ioQuake3-GC" $(if $(ISO_ROOT),--root $(ISO_ROOT))

#---------------------------------------------------------------------------------
else

DEPENDS	:=	$(OFILES:.o=.d)

all: $(OUTPUT).dol

$(BOTLIB_OBJ): CFLAGS += -DBOTLIB
code/qcommon/ioapi.o: CFLAGS += -DUSE_INTERNAL_ZLIB
$(ZLIB_OBJ): CFLAGS += -DUSE_INTERNAL_ZLIB -I$(REPO_ROOT)/code/zlib
$(JPEG_OBJ): CFLAGS += -DUSE_INTERNAL_JPEG -I$(REPO_ROOT)/code/jpeg-8c
code/renderer/tr_image_jpg.o: CFLAGS += -DUSE_INTERNAL_JPEG -I$(REPO_ROOT)/code/jpeg-8c

# Each module is partially linked, then all globals except its entry points and
# data/bss markers are hidden so it cannot resolve into the engine or another module.
define GC_MODULE_RULES
$(1)_OBJS := $$(patsubst %.c,$$(MODULE_DIR)/$(1)/%.o,$$($(1)_SRCS))

$$(MODULE_DIR)/$(1)/%.o: %.c
	@mkdir -p $$(dir $$@)
	@echo "CC [$(1)] $$<"
	@$$(CC) -MMD -MP -MF $(DEPSDIR)/$$(dir $$@)$$(notdir $$*).d $(CPPFLAGS) $$(CFLAGS) $(3) -fno-common -D$(2) \
		-DvmMain=vmMain_$(1) -DdllEntry=dllEntry_$(1) -c $$< -o $$@ $(ERROR_FILTER)

$$(MODULE_DIR)/$(1).o: $$($(1)_OBJS) $(REPO_ROOT)/code/sys/gc_module.ld
	@echo "MODULE $$@"
	@$$(PREFIX)ld -r -T $(REPO_ROOT)/code/sys/gc_module.ld -o $$@.r $$($(1)_OBJS)
	@$$(OBJCOPY) -G vmMain_$(1) -G dllEntry_$(1) -G gcmod_data_start -G gcmod_data_end \
		-G gcmod_bss_start -G gcmod_bss_end $$@.r $$@.g
	@$$(OBJCOPY) --redefine-sym gcmod_data_start=gcmod_$(1)_data_start \
		--redefine-sym gcmod_data_end=gcmod_$(1)_data_end \
		--redefine-sym gcmod_bss_start=gcmod_$(1)_bss_start \
		--redefine-sym gcmod_bss_end=gcmod_$(1)_bss_end $$@.g $$@
	@rm -f $$@.r $$@.g

-include $$($(1)_OBJS:.o=.d)
endef

$(eval $(call GC_MODULE_RULES,qagame,QAGAME,))
$(eval $(call GC_MODULE_RULES,cgame,CGAME,))
$(eval $(call GC_MODULE_RULES,ui,UI,-I$(REPO_ROOT)/code/q3_ui))

$(OUTPUT).dol: $(OUTPUT).elf
$(OUTPUT).elf: $(OFILES)

-include $(DEPENDS)

#---------------------------------------------------------------------------------
endif
