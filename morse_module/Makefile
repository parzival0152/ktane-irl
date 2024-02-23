compile:
	cd build/ && make

upload:
	sudo mount /dev/sda1 /mnt/pico
	sudo cp build/*.uf2 /mnt/pico
	sudo umount /mnt/pico

connect:
	sudo minicom -b 115200 -o -D /dev/ttyACM0
