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

SOURCES		:=	code/sys
CFILES		:=	gc_main.c
CPPFILES	:=
INCLUDES	:=

#---------------------------------------------------------------------------------
CFLAGS		=	-O2 -Wall $(MACHDEP) -G 0 -msdata=none -DHW_DOL $(FLAVOR_CFLAGS) $(DEBUG_CFLAGS) $(INCLUDE)

LDFLAGS		=	$(MACHDEP) -G 0 -msdata=none -Wl,-Map,$(OUTPUT).map $(DEBUG_LDFLAGS)

LIBS		:=	-logc -lm

LIBDIRS		:=

#---------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))

export OUTPUT	:=	$(CURDIR)/$(BUILD)/$(TARGET)

export VPATH	:=	$(foreach dir,$(SOURCES),$(CURDIR)/$(dir))

export DEPSDIR	:=	$(CURDIR)/$(BUILD)

export LD	:=	$(CC)

export OFILES	:=	$(CPPFILES:.cpp=.o) $(CFILES:.c=.o)

export INCLUDE	:=	$(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
			$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
			-I$(CURDIR)/$(BUILD) \
			-I$(LIBOGC_INC)

export LIBPATHS	:=	-L$(LIBOGC_LIB) $(foreach dir,$(LIBDIRS),-L$(dir)/lib)

.PHONY: all clean iso $(BUILD)

all: $(BUILD)

$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

clean:
	@echo clean ...
	@rm -rf build build_* *.gcm *.iso
	@$(MAKE) --no-print-directory -C tools/apploader clean

iso: all
	@$(MAKE) -C tools/apploader
	@python3 tools/mkgcm.py --dol $(BUILD)/$(TARGET).dol --apploader build/apploader/apploader.bin --out $(BUILD)/$(TARGET).gcm --id GQ3E --maker QG --name "ioQuake3-GC"

#---------------------------------------------------------------------------------
else

DEPENDS	:=	$(OFILES:.o=.d)

all: $(OUTPUT).dol

$(OUTPUT).dol: $(OUTPUT).elf
$(OUTPUT).elf: $(OFILES)

-include $(DEPENDS)

#---------------------------------------------------------------------------------
endif
