all: led
clean: clean-led

# Rules to build LED module
#
led:
	make -C build/led all

clean-led:
	make -C build/led clean

install-led: led
	make -C build/led install
