EXECUTABLE := raspberrypi-boot-mk2
SOURCE := ${EXECUTABLE}.c
UDEV_CONFIG := 99-raspberrypi.rules

### Building

all:
	gcc ${SOURCE} \
		lib/rpiusb.c -lusb-1.0 \
		-o ${EXECUTABLE}

dependencies:
	sudo apt install \
		libusb-1.0-0 \
		libusb-1.0-0-dev

clean:
	rm -f ${EXECUTABLE}

### Installation

install:
	sudo install -m 755 ${EXECUTABLE} /usr/local/bin/
	sudo install -m 644 udev/${UDEV_CONFIG} /etc/udev/rules.d/

uninstall:
	sudo rm -f /usr/local/bin/${EXECUTABLE}
	sudo rm -f /etc/udev/rules.d/${UDEV_CONFIG}
