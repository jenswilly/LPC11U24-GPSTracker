# LPC11U24 USB CDC example
This project creates a [USB Communications Device Class](http://en.wikipedia.org/wiki/USB_communications_device_class) – i.e. a 
virtual serial port.

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

## Code walkthrough
All the interesting stuff happens in `main.c`.

### Initialization
`init_uart1_bridge()`  
`USB_pin_clk_init()`  
`main()`

### UART methods
(Not including init methods:)  
`UART_IRQHandler()`   
`uart_write()`  
`uart_read()`  
`UARTSend()`  
Method for sending a string on UART without relying on VCOM_DATA structs.
Example: `UARTSend( (uint8_t*)"USB connect\r\n", 13);`


### USB methods
(Again, not including init methods:)  
`VCOM_bulk_in_hdlr()`  
`VCOM_bulk_out_hdlr()`  
`VCOM_SendBreak()`  
`VCOM_SetLineCode()`  
`VCOM_sof_event()`  
`VCOM_uart_send()`  
`VCOM_usb_send()`