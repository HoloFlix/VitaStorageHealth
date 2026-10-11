TITLE_ID := VSHL00001
TITLE    := Vita Storage Health
TARGET   := VitaStorageHealth
OBJS     := src/main.o src/storage.o src/benchmark.o src/integrity.o src/tools.o src/plugins.o src/ui.o

LIBS = -lvita2d -lpng -lz -lScePgf_stub -lSceDisplay_stub -lSceGxm_stub \
	-lSceSysmodule_stub -lSceCtrl_stub -lSceTouch_stub \
	-lSceAppMgr_stub -lScePower_stub -lSceRtc_stub \
	-lSceCommonDialog_stub -lSceAppUtil_stub -lm -lc

PREFIX  = arm-vita-eabi
CC      = $(PREFIX)-gcc
CFLAGS  = -Wl,-q -Wall -O3 -std=c99
ASFLAGS = $(CFLAGS)

all: $(TARGET).vpk

param.sfo:
	vita-mksfoex -s TITLE_ID="$(TITLE_ID)" "$(TITLE)" param.sfo

%.vpk: %.velf param.sfo
	vita-make-fself -c $< eboot.bin
	vita-pack-vpk -s param.sfo -b eboot.bin -a sce_sys=sce_sys -a art=art $@

%.velf: %.elf
	vita-elf-create $< $@

$(TARGET).elf: $(OBJS)
	$(CC) $(CFLAGS) $^ $(LIBS) -o $@

clean:
	@rm -rf $(TARGET).vpk $(TARGET).velf $(TARGET).elf $(OBJS) eboot.bin param.sfo

.PHONY: all clean
