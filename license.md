# Licenses
This project uses source code from several different sources. All of them are unde some sort of "free to use and distribute" license but terms and conditions vary wildly ([here's a brief comparison](http://en.wikipedia.org/wiki/Comparison_of_free_and_open-source_software_licenses)).

In this document I'll try to keep track of the licences, terms and conditions.

I will make an earnest effort to adhere to all applicable terms and conditions. If I have forgotten to include a copyright notice or an attribution somewhere let me know and I'll fix it ASAP.

## My code
All my original code is released under **Creative Commons – Attribution 3.0 Unported (CC BY 3.0)** license: use it and change it as you wish but leave a comment in the source code attributing me ("Jens Willy Johannsen <jens@jenswilly.dk>") as the original author.  
Read the full license term and conditions here: <http://creativecommons.org/licenses/by/3.0/>

## Code from NXP Application Notes
Source code from NXP's Application Notes are relased under the following terms:

	****************************************************************************  
	* Software that is described herein is for illustrative purposes only  
	* which provides customers with programming information regarding the  
	* products. This software is supplied "AS IS" without any warranties.  
	* NXP Semiconductors assumes no responsibility or liability for the  
	* use of the software, conveys no license or title under any patent,  
	* copyright, or mask work right to the product. NXP Semiconductors  
	* reserves the right to make changes in the software without  
	* notification. NXP Semiconductors also make no representation or
	* warranty that such application will be suitable for the specified
	* use without further testing or modification.
	*  
	* Permission to use, copy, modify, and distribute this software and its 
	* documentation is hereby granted, under NXP Semiconductorsí 
	* relevant copyright in the software, without fee, provided that it 
	* is used in conjunction with NXP Semiconductors microcontrollers.  This 
	* copyright, permission, and disclaimer notice must appear in all copies of 
	* this code.
	****************************************************************************/

Basically, the code can be distrbuted and modified but the code should be used with NXP MCUs and the above notice should be included.

This includes:

- `/drivers/uart/`. The code has been modified by me.
- `/drivers/eeprom/` AN11073.
- `/drivers/soft_uart/` AN10955. Code has been modified by me.
- `/drivers/usb/` from [USB ROM Driver examples using LPCXpresso for LPC11Uxx](http://www.lpcware.com/content/nxpfile/usb-rom-driver-examples-using-lpcxpresso-lpc11uxx).
- `/drivers/usb_cdc/` from [USB ROM Driver examples using LPCXpresso for LPC11Uxx](http://www.lpcware.com/content/nxpfile/usb-rom-driver-examples-using-lpcxpresso-lpc11uxx). Code has been modified by me.

## Device files
The files in `/cmsis/core/` and the `system_LPC11Uxx` files in `/cmsis/device/` are distributed with the following copyright notice and disclaimer:

	@note
	Copyright (C) 2009-2010 ARM Limited. All rights reserved.
	
	@par
	ARM Limited (ARM) is supplying this software for use with Cortex-M 
	processor based microcontrollers.  This file can be freely distributed 
	within development tools that are supporting such ARM based processors. 
	
	@par
	THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
	OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
	MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
	ARM SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR
	CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
