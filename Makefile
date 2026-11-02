# --- SNES / BS-X / Sufami Turbo Makefile ---
ROMNAME = snes_maze

# PVSnesLib kök dizini (Eğer ortam değişkeni yoksa varsayılanı kullanır)
ifeq ($(strip $(PVSNESLIB)),)
PVSNESLIB = /opt/pvsneslib
export PVSNESLIB
endif

# Kaynak dosyaları
CFILES = $(ROMNAME).c

# PVSnesLib standart derleme kurallarını dahil et
include $(PVSNESLIB)/devkitsnes/snes_rules

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
