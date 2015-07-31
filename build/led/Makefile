SRCDIR = ../../src

MODULETYPE = led
MODULECFLAGS = -DOBJECT_LED
MODULESRC = main.c clock.c uart.c rtimer.c led.c radio.c json.c jsmn.c
MODULEREL = $(MODULESRC:.c=.rel)

SDCCCFLAGS = --model-large --opt-code-size --no-xinit-opt --std-c99
ASLINKFLAGS = --model-large --code-loc 0x0000 --xram-loc 0x0000 --xram-size 0x2000

%.rel : $(SRCDIR)/%.c
	sdcc $(SDCCCFLAGS) $(MODULECFLAGS) -c -mmcs51 $<

all: $(MODULETYPE).bin

$(MODULETYPE).bin: $(MODULEREL)
	sdcc -o $(MODULETYPE).ihx $(MODULECFLAGS) $(SDCCCFLAGS) $(ASLINKFLAGS) $^
	packihx $(MODULETYPE).ihx > $(MODULETYPE).hex
	srec_cat $(MODULETYPE).hex -intel -o $(MODULETYPE).bin -binary

clean:
	rm -f *.asm *.bin *.hex *.ihx *.lk *.lst *.map *.mem *.rel *.rst *.sym

install: $(MODULETYPE).bin
	cc-tool -ew $(MODULETYPE).bin
