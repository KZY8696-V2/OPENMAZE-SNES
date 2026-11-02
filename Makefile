# --- SNES / BS-X / Sufami Turbo Makefile ---
ROMNAME = snes_maze

# PVSnesLib Kesin Yolları (Çift katmanlı veya tek katmanlı yapıları destekler)
export PVSNESLIB ?= /opt/pvsneslib
export DEVKITSNES ?= $(PVSNESLIB)/devkitsnes

CFILES = $(ROMNAME).c

# Derleyicinin (816-tcc) başlık dosyalarını bulabilmesi için arama yollarını garantiye al
export INCLUDE := $(DEVKITSNES)/include:$(PVSNESLIB)/include:$(PVSNESLIB)/pvsneslib/include:/pvsneslib/include:/devkitsnes/include
export LIB := $(DEVKITSNES)/lib:$(PVSNESLIB)/lib:$(PVSNESLIB)/pvsneslib/lib:/pvsneslib/lib:/devkitsnes/lib

# Derleme öncesi kök dizin linklerini kesin olarak tazeleyin
ifeq ($(wildcard /pvsneslib),)
$(shell sudo ln -sfn $(PVSNESLIB) /pvsneslib)
endif
ifeq ($(wildcard /devkitsnes),)
$(shell sudo ln -sfn $(DEVKITSNES) /devkitsnes)
endif

# PVSnesLib kurallarını dahil et
include $(DEVKITSNES)/snes_rules

# Hedefler (NOT: Komut satırlarının başında mutlaka TAB olmalıdır)
all: $(ROMNAME).sfc
	@echo "Tüm hedef platformlar için ROM uzantıları oluşturuluyor..."
	@cp $(ROMNAME).sfc $(ROMNAME)_snes.sfc
	@cp $(ROMNAME).sfc $(ROMNAME)_superfamicom.smc
	@cp $(ROMNAME).sfc $(ROMNAME)_bsx.bsx
	@cp $(ROMNAME).sfc $(ROMNAME)_sufamiturbo.sft
	@echo "Tüm platform ROM'ları başarıyla hazırlandı!"

clean:
	@rm -f *.o *.s *.asm *.sym *.map *.lst *.sfc *.smc *.bsx *.sft
