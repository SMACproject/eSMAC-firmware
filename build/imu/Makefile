SRCDIR = ../../src

MODULETYPE = imu
MODULECFLAGS = -DOBJECT_LED1 -DOBJECT_IMU -DSPI1_ENABLE
MODULESRC = main.c clock.c uart.c spi.c spi1.c spi-arch.c flash.c rtimer.c radio.c json.c jsmn.c
MODULESRC += led.c lsm9ds0.c
MODULEREL = $(MODULESRC:.c=.rel)

#
# DO NOT CHANGE ANYTHING BEYOND THIS LINE, UNLESS YOU KNOW WHAT YOU ARE DOING
#

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
