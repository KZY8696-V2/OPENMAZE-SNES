# --- SNES / BS-X / Sufami # --- SNES / BS-X / Sufami Turbo Makefile ---
ROMNAME = snes_maze

# PVSnesLib Kesin Yolları
export PVSNESLIB = /opt/pvsneslib
export DEVKITSNES = $(PVSNESLIB)/devkitsnes

CFILES = $(ROMNAME).c

# Include ve Lib yollarını PVSnesLib'in gerçek konumuna göre override et
export INCLUDE := $(PVSNESLIB)/include:$(DEVKITSNES)/include
export LIB := $(PVSNESLIB)/lib:$(DEVKITSNES)/lib

# Derleme öncesi /devkitsnes kök linkini garantiye al
ifeq ($(wildcard /devkitsnes),)
$(shell sudo ln -sfn $(DEVKITSNES) /devkitsnes)
endif
ifeq ($(wildcard /pvsneslib),)
$(shell sudo ln -sfn $(PVSNESLIB) /pvsneslib)
endif

# PVSnesLib kurallarını dahil et
include $(DEVKITSNES)/snes_rules

# Hedefler
all: $(ROMNAME).sfc
	@echo "Tüm hedef platformlar için ROM uzantıları oluşturuluyor..."
	@cp $(ROMNAME).sfc $(ROMNAME)_snes.sfc
	@cp $(ROMNAME).sfc $(ROMNAME)_superfamicom.smc
	@cp $(ROMNAME).sfc $(ROMNAME)_bsx.bsx
	@cp $(ROMNAME).sfc $(ROMNAME)_sufamiturbo.sft
	@echo "Tüm platform ROM'ları başarıyla hazırlandı!"

clean:
	@rm -f *.o *.s *.asm *.sym *.map *.lst *.sfc *.smc *.bsx *.sft
 Makefile ---
ROMNAME = snes_maze

export PVSNESLIB = /opt/pvsneslib
export DEVKITSNES = $(PVSNESLIB)/devkitsnes

CFILES = $(ROMNAME).c

# Derleme öncesi /devkitsnes kök linkinin varlığını garantiye al
ifeq ($(wildcard /devkitsnes),)
$(shell sudo ln -s $(DEVKITSNES) /devkitsnes)
endif

include $(DEVKITSNES)/snes_rules

all: $(ROMNAME).sfc
	@echo "Tüm hedef platformlar için ROM uzantıları oluşturuluyor..."
	@cp $(ROMNAME).sfc $(ROMNAME)_snes.sfc
	@cp $(ROMNAME).sfc $(ROMNAME)_superfamicom.smc
	@cp $(ROMNAME).sfc $(ROMNAME)_bsx.bsx
	@cp $(ROMNAME).sfc $(ROMNAME)_sufamiturbo.sft
	@echo "Tüm platform ROM'ları başarıyla hazırlandı!"

clean:
	@rm -f *.o *.s *.asm *.sym *.map *.lst *.sfc *.smc *.bsx *.sft
