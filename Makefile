all: bluetooth led motor imu
clean: clean-bluetooth clean-led clean-motor clean-imu

# Rules to build bluetooth module
#
bluetooth:
	@echo "*** BUILDING BLUETOOTH MODULE ***"
	make -C build/bluetooth all

clean-bluetooth:
	make -C build/bluetooth clean

install-bluetooth: bluetooth
	make -C build/bluetooth install


# Rules to build LED module
#
led:
	@echo "*** BUILDING LED MODULE ***"
	make -C build/led all

clean-led:
	make -C build/led clean

install-led: led
	make -C build/led install


# Rules to build motor module
#
motor:
	@echo "*** BUILDING MOTOR MODULE ***"
	make -C build/motor all

clean-motor:
	make -C build/motor clean

install-motor: motor
	make -C build/motor install


# Rules to build IMU module
#
imu:
	@echo "*** BUILDING IMU MODULE ***"
	make -C build/imu all

clean-imu:
	make -C build/imu clean

install-imu: imu
	make -C build/imu install
