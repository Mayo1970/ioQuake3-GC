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
			q_shared.o unzip.o vm.o vm_interpreted.o)

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

NULL_OBJ	:=	code/null/null_input.o

SYS_OBJ		:=	$(addprefix code/sys/,gc_main.o sys_gc.o gc_dvd.o \
			gc_null_renderer.o gc_null_snddma.o gc_net_null.o)

ZLIB_CFILES	:=	$(notdir $(wildcard $(CURDIR)/code/zlib/*.c))
ZLIB_OBJ	:=	$(addprefix code/zlib/,$(ZLIB_CFILES:.c=.o))

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
			$(CLIENT_OBJ) $(NULL_OBJ) $(SYS_OBJ) $(ZLIB_OBJ)

export BOTLIB_OBJ
export ZLIB_OBJ
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

$(OUTPUT).dol: $(OUTPUT).elf
$(OUTPUT).elf: $(OFILES)

-include $(DEPENDS)

#---------------------------------------------------------------------------------
endif
