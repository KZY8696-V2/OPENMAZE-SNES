# --- SNES / BS-X / Sufami Turbo Makefile ---
ROMNAME = snes_maze

# Ayrı ayrı konumlandırılmış kararlı yollar
export PVSNESLIB = /opt/pvsneslib
export DEVKITSNES = /opt/devkitsnes

CFILES = $(ROMNAME).c

# PVSnesLib derleme kurallarını dahil et
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
