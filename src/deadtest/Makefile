
.PHONY: all clean

PASMO = pasmo
PASMO_OPTIONS = --bin

SOURCES = deadtest.asm

all: vzdt8k.rom vzdt16k.rom vzdt32k.rom

vzdt8k.rom: $(SOURCES)
	$(PASMO) $(PASMO_OPTIONS) deadtest.asm vzdt8k.rom

vzdt16k.rom: vzdt8k.rom
	cat vzdt8k.rom vzdt8k.rom >vzdt16k.rom

vzdt32k.rom: vzdt16k.rom
	cat vzdt16k.rom vzdt16k.rom >vzdt32k.rom

clean:
	rm -f *.rom
