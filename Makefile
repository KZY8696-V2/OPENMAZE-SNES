# --- SNES / BS-X / Sufami Turbo Birleştirilmiş Makefile ---
target = snes_maze

# PVSnesLib Konumu (Docker konteyneri içi veya sistem yolu)
PVSNESLIB ?= /opt/pvsneslib

include $(PVSNESLIB)/devkitsnes/snes_rules

# Varsayılan olarak ana ROM'u derle ve tüm platform uzantılarına kopyala
all: $(target).sfc
	@echo "Tüm hedef platformlar için ROM uzantıları oluşturuluyor..."
	@cp $(target).sfc $(target)_snes.sfc
	@cp $(target).sfc $(target)_superfamicom.smc
	@cp $(target).sfc $(target)_bsx.bsx
	@cp $(target).sfc $(target)_sufamiturbo.sft
	@echo "Tüm platform ROM'ları başarıyla hazırlandı!"

# Klasik ve kararlı PVSnesLib bağlama kuralı
$(target).sfc: $(target).o
	$(LD) -o $@ $(LDFLAGS) $^

# Temizleme Kuralı
clean:
	@rm -f *.o *.s *.asm *.sym *.map *.lst *.sfc *.smc *.bsx *.sft
