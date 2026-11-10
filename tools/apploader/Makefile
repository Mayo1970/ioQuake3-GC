DEVKITPPC ?= /opt/devkitpro/devkitPPC

PREFIX	:=	$(DEVKITPPC)/bin/powerpc-eabi-
CC	:=	$(PREFIX)gcc
OBJCOPY	:=	$(PREFIX)objcopy

TARGET	:=	apploader
BUILD	:=	../../build/apploader

CFLAGS	:=	-mogc -mcpu=750 -meabi -mhard-float -Os -ffreestanding -nostdlib -fno-builtin -G 0 -msdata=none
LDFLAGS	:=	-T $(CURDIR)/apploader.ld
LIBS	:=	-lgcc

.PHONY: all clean

all: $(BUILD)/$(TARGET).bin

$(BUILD)/$(TARGET).bin: $(BUILD)/$(TARGET).elf
	$(OBJCOPY) -O binary $< $@

$(BUILD)/$(TARGET).elf: apploader.c apploader.ld | $(BUILD)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ apploader.c $(LIBS)

$(BUILD):
	@[ -d $@ ] || mkdir -p $@

clean:
	@rm -rf $(BUILD)
