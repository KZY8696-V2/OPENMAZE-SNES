# --- SNES / BS-X / Sufami Turbo Makefile ---
ROMNAME = snes_maze

# PVSnesLib Konumu ve Toolchain Yolları Kesin Tanımı
export PVSNESLIB ?= /opt/pvsneslib
export DEVKITSNES = $(PVSNESLIB)/devkitsnes

# Kaynak C Dosyası
CFILES = $(ROMNAME).c

# PVSnesLib standart derleme kuralları
include $(DEVKITSNES)/snes_rules

# Varsayılan hedef: ROM'u derle ve tüm çoklu platform uzantılarını oluştur
all: $(ROMNAME).sfc
	@echo "Tüm hedef platformlar için ROM uzantıları oluşturuluyor..."
	@cp $(ROMNAME).sfc $(ROMNAME)_snes.sfc
	@cp $(ROMNAME).sfc $(ROMNAME)_superfamicom.smc
	@cp $(ROMNAME).sfc $(ROMNAME)_bsx.bsx
	@cp $(ROMNAME).sfc $(ROMNAME)_sufamiturbo.sft
	@echo "Tüm platform ROM'ları başarıyla hazırlandı!"

# Temizleme Kuralı
clean:
	@rm -f *.o *.s *.asm *.sym *.map *.lst *.sfc *.smc *.bsx *.sft
