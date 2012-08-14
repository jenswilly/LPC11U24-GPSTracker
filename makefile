##############################################################################################
#
#  Makefile
#  
#  Created by Jens Willy Johannsen on 2012-08-14.
#  
#  This file is released under Creative Commons – Attribution 3.0 Unported (CC BY 3.0)
#  http://creativecommons.org/licenses/by/3.0/
#
#  This file is based on the Makefile found here: http://www.codeforge.com/read/77250/makefile__html
#  
##############################################################################################
# 
#  Available targets:
#
#  make all = Build elf file
#  make bin = Build and convert to binary file
#  make crc = Build, convert to binary and add checksum
#  make flash = Build, convert to binary, add checksum and copy to USB device
#  make size = Show size usage
#  make objdump = Dump objects to objdump.txt
#  make clean = Clean project
#
##############################################################################################

# Define project name and MCU here
PROJECT        = gpstracker
MCU			   = cortex-m0
SYSCALLS	   = 0

# Use '.' for no common dir. Do not use trailing slash
COMMONDIR	   = ./common_LPC11U24

# Define linker script file here
LDSCRIPT = $(COMMONDIR)/LPC11U24.ld

# List all user C define here, like -D_DEBUG=1
UDEFS = -D__USE_CMSIS

# Define ASM defines here
UADEFS = 

# List C source files here
SRC  = $(COMMONDIR)/cmsis/core/core_cm0.c \
       $(COMMONDIR)/cmsis/device/system_LPC11Uxx.c \
       $(COMMONDIR)/cmsis/device/startup_LPC11U24.c \
	   $(COMMONDIR)/drivers/usb/cdc_desc.c \
	   $(COMMONDIR)/drivers/usb_cdc/usb_cdc.c \
	   $(COMMONDIR)/drivers/eeprom/eeprom.c \
       ./src/main.c

# List ASM source files here
ASRC =

# List all user directories here
UINCDIR = ./src \
          $(COMMONDIR)/cmsis/core \
          $(COMMONDIR)/cmsis/device \
          $(COMMONDIR)/drivers/eeprom \
          $(COMMONDIR)/drivers/usb_cdc \
		  $(COMMONDIR)/drivers/usb

# List the user directory to look for the libraries here
ULIBDIR =

# List all user libraries here
ULIBS = 

# Define optimisation level here
OPT = -O1

#
# End of configuration section
##############################################################################################

TRGT = arm-none-eabi-
CC   = $(TRGT)gcc
CP   = $(TRGT)objcopy
AS   = $(TRGT)gcc -x assembler-with-cpp
BIN  = $(CP) -O ihex 
ECHO = echo
CAT  = cat

# Use colors? (1/0)
USE_COLORS = 1

# List all default C defines here, like -D_DEBUG=1
DDEFS =

# List all default ASM defines here, like -D_DEBUG=1
DADEFS = 

# List all default directories to look for include files here
DINCDIR = 

# List the default directory to look for the libraries here
DLIBDIR =

# List all default libraries here
DLIBS = 

# Color definitions

NO_COLOR=\x1b[0m
OK_COLOR=\x1b[32;01m
ERROR_COLOR=\x1b[31;01;47m
WARN_COLOR=\x1b[33;01m

OK_STRING=$(OK_COLOR)[OK]$(NO_COLOR)
ERROR_STRING=$(ERROR_COLOR)[ERRORS]$(NO_COLOR)
WARN_STRING=$(WARN_COLOR)[WARNINGS]$(NO_COLOR)
DONE_STRING=$(OK_COLOR)[Done]$(NO_COLOR)

# Use these color definitions for Eclipse projects
#OK_STRING=[OK]
#ERROR_STRING=[ERRORS]
#WARN_STRING=[WARNINGS]
#DONE_STRING=[Done]

# End of color definitions
###

ifeq ($(SYSCALLS),1)
	SRC += ./src/syscalls.c
endif

INCDIR  = $(patsubst %,-I%,$(DINCDIR) $(UINCDIR))
LIBDIR  = $(patsubst %,-L%,$(DLIBDIR) $(ULIBDIR))

DEFS    = $(DDEFS) $(UDEFS) -DRUN_FROM_FLASH=1
ADEFS   = $(DADEFS) $(UADEFS)
OBJS    = $(ASRC:.s=.o) $(SRC:.c=.o)
LIBS    = $(DLIBS) $(ULIBS)
MCFLAGS = -mcpu=$(MCU)

ASFLAGS = $(MCFLAGS) -Wa,-amhls=$(<:.s=.lst) $(ADEFS)
CPFLAGS = $(MCFLAGS) $(OPT) -mthumb -fomit-frame-pointer -Wall -Wstrict-prototypes -fverbose-asm -Wa,-ahlms=$(<:.c=.lst) $(DEFS)
LDFLAGS = $(MCFLAGS) -mthumb -nostartfiles -T$(LDSCRIPT) -Wl,-Map=$(PROJECT).map,--cref,--no-warn-mismatch $(LIBDIR)

# See if we have the USB device attached and mounted (is used in the flash target)
DEVICE = $(shell df -h | awk '/CRP DISABLD$$/ {print $$1}')

# Generate dependency information
CPFLAGS += -MD -MP -MF .dep/$(@F).d

##############################################################################################
# Rules section
#

all: $(OBJS) $(PROJECT).elf $(PROJECT).hex

bin: $(OBJS) $(PROJECT).bin size clean-intermediates

size:
	@$(ECHO) Size:
	@arm-none-eabi-size $(PROJECT).elf

objdump:
	arm-none-eabi-objdump -x -d $(PROJECT).elf >objdump.txt

crc: bin
ifeq ($(USE_COLORS),1)
	@$(ECHO) -n Adding checksum to $(PROJECT).bin...
	@crc $(PROJECT).bin 2> temp.log || touch temp.errors
	@if test -e temp.errors; then $(ECHO) "$(ERROR_STRING)" && $(CAT) temp.log; elif test -s temp.log; then $(ECHO) "$(WARN_STRING)" && $(CAT) temp.log; else $(ECHO) "$(DONE_STRING)"; fi;
	@rm -f temp.errors temp.log
else
	crc $(PROJECT).bin 
endif

clean-intermediates:
	@$(ECHO) -n Removing intermediates... 
	@rm -f $(OBJS)
	@rm -f $(PROJECT).elf
	@rm -f $(SRC:.c=.c.bak)
	@rm -f $(SRC:.c=.lst)
	@rm -f $(ASRC:.s=.s.bak)
	@rm -f $(ASRC:.s=.lst)
	@rm -fR .dep
ifeq ($(USE_COLORS),1)
	@$(ECHO) "$(DONE_STRING)"
else
	@$(ECHO) Done
endif

clean:
	@$(ECHO) -n Cleaning...
	@rm -f $(OBJS)
	@rm -f $(PROJECT).elf
	@rm -f $(PROJECT).map
	@rm -f $(PROJECT).hex
	@rm -f $(PROJECT).bin
	@rm -f $(SRC:.c=.c.bak)
	@rm -f $(SRC:.c=.lst)
	@rm -f $(ASRC:.s=.s.bak)
	@rm -f $(ASRC:.s=.lst)
	@rm -fR .dep
ifeq ($(USE_COLORS),1)
	@$(ECHO) "$(DONE_STRING)"
else
	@$(ECHO) Done
endif

flash: crc
	$(if $(DEVICE),\
	 @$(ECHO) -n Unmounting $(DEVICE)...; \
	 diskutil unmount $(DEVICE); \
	 $(ECHO) "$(OK_STRING)";\
	 $(ECHO) -n Flashing $(PROJECT).bin to $(DEVICE)...; \
	 dd if=$(PROJECT).bin of=$(DEVICE) seek=4; \
	 $(ECHO) "$(OK_STRING)",\
	@$(ECHO) "$(ERROR_STRING) Device not found")

#
# File rules
#

%.o : %.c
ifeq ($(USE_COLORS),1)
	@$(ECHO) -n Compiling $<...
	@$(CC) -c $(CPFLAGS) -I . $(INCDIR) $< -o $@ 2> temp.log || touch temp.errors
	@if test -e temp.errors; then $(ECHO) "$(ERROR_STRING)" && $(CAT) temp.log; elif test -s temp.log; then $(ECHO) "$(WARN_STRING)" && $(CAT) temp.log; else $(ECHO) "$(OK_STRING)"; fi;
	@rm -f temp.errors temp.log
else
	$(CC) -c $(CPFLAGS) -I . $(INCDIR) $< -o $@
endif

%.o : %.s
	$(AS) -c $(ASFLAGS) $< -o $@

%elf: $(OBJS)
ifeq ($(USE_COLORS),1)
	@$(ECHO) -n Linking $(OBJS)...
	@$(CC) $(OBJS) $(LDFLAGS) $(LIBS) -o $@ 2> temp.log || touch temp.errors
	@if test -e temp.errors; then $(ECHO) "$(ERROR_STRING)" && $(CAT) temp.log; elif test -s temp.log; then $(ECHO) "$(WARN_STRING)" && $(CAT) temp.log; else $(ECHO) "$(OK_STRING)"; fi;
	@rm -f temp.errors temp.log
else
	$(CC) $(OBJS) $(LDFLAGS) $(LIBS) -o $@
endif
  
%hex: %elf
ifeq ($(USE_COLORS),1)
	@$(ECHO) -n Copying $< to $@...
	@$(BIN) $< $@ 2> temp.log || touch temp.errors
	@if test -e temp.errors; then $(ECHO) "$(ERROR_STRING)" && $(CAT) temp.log; elif test -s temp.log; then $(ECHO) "$(WARN_STRING)" && $(CAT) temp.log; else $(ECHO) "$(OK_STRING)"; fi;
	@rm -f temp.errors temp.log
else
	$(BIN) $< $@
endif

%bin: %elf
ifeq ($(USE_COLORS),1)
	@$(ECHO) -n Copying $< to $@...
	@$(CP) -O binary $< $@ 2> temp.log || touch temp.errors
	@if test -e temp.errors; then $(ECHO) "$(ERROR_STRING)" && $(CAT) temp.log; elif test -s temp.log; then $(ECHO) "$(WARN_STRING)" && $(CAT) temp.log; else $(ECHO) "$(OK_STRING)"; fi;
	@rm -f temp.errors temp.log
else
	$(CP) -O binary $< $@ 
endif


# 
# Include the dependency files, should be the last of the makefile
#
-include $(shell mkdir .dep 2>/dev/null) $(wildcard .dep/*)
