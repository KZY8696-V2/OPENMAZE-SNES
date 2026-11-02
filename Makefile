# --- SNES / BS-X / Sufami Turbo Makefile ---
ROMNAME = snes_maze

# PVSnesLib Kesin Yolları
export PVSNESLIB = /opt/pvsneslib
export DEVKITSNES = $(PVSNESLIB)/devkitsnes

CFILES = $(ROMNAME).c

# Derleme öncesi kök dizindeki /pvsneslib ve /devkitsnes bağlantılarını kesin olarak oluştur
# Bu sayede 816-tcc derleyicisi /pvsneslib/include yolunu bulabilecektir.
$(shell sudo rm -rf /pvsneslib /devkitsnes)
$(shell sudo ln -sfn $(PVSNESLIB) /pvsneslib)
$(shell sudo ln -sfn $(DEVKITSNES) /devkitsnes)

# PVSnesLib standart derleme kurallarını dahil et
include $(DEVKITSNES)/snes_rules

# Hedefler (Not: Satır başlarında mutlaka TAB olmalıdır)
all: $(ROMNAME).sfc
	@echo "Tüm hedef platformlar için ROM uzantıları oluşturuluyor..."
	@cp $(ROMNAME).sfc $(ROMNAME)_snes.sfc
	@cp $(ROMNAME).sfc $(ROMNAME)_superfamicom.smc
	@cp $(ROMNAME).sfc $(ROMNAME)_bsx.bsx
	@cp $(ROMNAME).sfc $(ROMNAME)_sufamiturbo.sft
	@echo "Tüm platform ROM'ları başarıyla hazırlandı!"

clean:
	@rm -f *.o *.s *.asm *.sym *.map *.lst *.sfc *.smc *.bsx *.sft
