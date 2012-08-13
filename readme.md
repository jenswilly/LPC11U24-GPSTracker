# LPC11U24 GPS Tracker
This project if firmware for a GSM enabled GPS Tracker.

The project uses CMSIS and compiles with the [Yagarto toolchain](http://yagarto.de).

## Building
This project uses the my standard Makefile. The following make targets are available:

	make all = Build elf file
	make bin = Build and convert to binary file. Also shows size information and removes intermediate files
	make crc = Build, convert to binary and add checksum
	make flash = Build, convert to binary, add checksum and copy to USB device
	make size = Show size usage
	make objdump = Dump objects and memory map to objdump.txt
	make clean = Clean project files.

### Flashing from Makefile
The `make flash` will attempt to find a filesystem mounted as `CRP DISABLD`, then unmount the filesystem and copy the binary data to the device starting at sector 4. In order to use it, do this:

1. Connect MCU and turn on in USB ISP mode.
2. Wait for the `CRP DISABLD` disk to be mounted.
3. `make flash`
4. Reset MCU

## Eclipse issues
In order to work properly as an Eclipse project (Makefile Project with Existing Code), the following changes have been made:

1. Project -> Properties -> C/C++ General -> Paths and Symbols -> Includes  
Added `/Users/jenswilly/Yagarto/yagarto-4.7.1/arm-none-eabi/include`
2. Project -> Properties -> C/C++ General -> Paths and Symbols -> Symbols  
Added symbol `__GNUC__` with value `1`

The changes are not necessary for building, but they *are* necessary for indexing and syntax check to work correctly.
