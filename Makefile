# --- SNES / BS-X / Sufami Turbo Makefile ---
ROMNAME = snes_maze

# PVSnesLib Konumu (Ubuntu runner üzerinde ayarlı)
PVSNESLIB ?= /opt/pvsneslib

include $(PVSNESLIB)/devkitsnes/snes_rules

# Varsayılan olarak ana ROM'u derle ve tüm platform uzantılarına kopyala
all: $(ROMNAME).sfc
	@echo "Tüm hedef platformlar için ROM uzantıları oluşturuluyor..."
	@cp $(ROMNAME).sfc $(ROMNAME)_snes.sfc
	@cp $(ROMNAME).sfc $(ROMNAME)_superfamicom.smc
	@cp $(ROMNAME).sfc $(ROMNAME)_bsx.bsx
	@cp $(ROMNAME).sfc $(ROMNAME)_sufamiturbo.sft
	@echo "Tüm platform ROM'ları başarıyla hazırlandı!"

# PVSnesLib standart bağlama kuralı
$(ROMNAME).sfc: $(ROMNAME).o
	$(LD) -o $@ $(LDFLAGS) $^

# Temizleme Kuralı
clean:
	@rm -f *.o *.s *.asm *.sym *.map *.lst *.sfc *.smc *.bsx *.sft
