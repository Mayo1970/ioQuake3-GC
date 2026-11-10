#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------
ifeq ($(strip $(DEVKITPPC)),)
$(error "Please set DEVKITPPC in your environment. export DEVKITPPC=<path to>devkitPPC")
endif

include $(DEVKITPPC)/gamecube_rules

#---------------------------------------------------------------------------------
TARGET		:=	systest
BUILD		:=	build
SOURCES		:=	source ../../code/sys ../../code/qcommon
INCLUDES	:=	../../code/qcommon ../../code/sys

#---------------------------------------------------------------------------------
CFLAGS		=	-O2 -Wall -Wno-char-subscripts $(MACHDEP) -G 0 -msdata=none -DBASEGAME=\"baseq3\" $(INCLUDE)

ifneq ($(DEBUG),)
CFLAGS		+=	-DGC_DEBUG -g
endif

ifeq ($(MODE),wait)
CFLAGS		+=	-DSYSTEST_WAIT
endif

LDFLAGS		=	$(MACHDEP) -G 0 -msdata=none -Wl,-Map,$(OUTPUT).map

LIBS		:=	-lfat -logc -lm

LIBDIRS		:=

#---------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))

export OUTPUT	:=	$(CURDIR)/$(TARGET)

export VPATH	:=	$(foreach dir,$(SOURCES),$(CURDIR)/$(dir))

export DEPSDIR	:=	$(CURDIR)/$(BUILD)

export CFILES	:=	main.c shims.c sys_gc.c q_shared.c
export CPPFILES	:=

export LD	:=	$(CC)

export OFILES	:=	$(CFILES:.c=.o)

export INCLUDE	:=	$(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
			$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
			-I$(CURDIR)/$(BUILD) \
			-I$(LIBOGC_INC)

export LIBPATHS	:=	-L$(LIBOGC_LIB) $(foreach dir,$(LIBDIRS),-L$(dir)/lib)

.PHONY: $(BUILD) clean

$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

clean:
	@echo clean ...
	@rm -fr $(BUILD) $(OUTPUT).elf $(OUTPUT).dol $(OUTPUT).map

#---------------------------------------------------------------------------------
else

DEPENDS	:=	$(OFILES:.o=.d)

$(OUTPUT).dol: $(OUTPUT).elf
$(OUTPUT).elf: $(OFILES)

-include $(DEPENDS)

#---------------------------------------------------------------------------------
endif
