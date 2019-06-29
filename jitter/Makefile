#	Makefile - for investigating jitter on raspberry pi (zero, 3)
#
#	Copyright 2019 David Haworth
#
#	This file is part of Dave's determinism experiments.
#
#	The experiments are free software: you can redistribute it and/or modify
#	it under the terms of the GNU General Public License as published by
#	the Free Software Foundation, either version 3 of the License, or
#	(at your option) any later version.
#
#	The experiments are distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#	GNU General Public License for more details.
#
#	You should have received a copy of the GNU General Public License
#	along with Dave's determinism experiments.
#	If not, see <http://www.gnu.org/licenses/>.
#	Usage:
#		make [BOARD=pi3-arm64|pi-zero] [GNU_D=</path/to/gcc>] [INSTALL_DIR=</place/to/install/]
#	Alternatively, you can set BOARD GNU_D and INSTALL_DIR as environment variables.
#
#	Targets:
#		clean: removes all object and binary files
#		default: compiles and links
#		install: objcopy the ELF file to a binary (img) file in INSTALL_DIR
#		srec: objcopy the ELF to an S-record file in the bin directory

# Find out where we are :-)
DV_ROOT		= ../../davros
DVSK_ROOT	= $(DV_ROOT)/davroska

# Select your hardware here
BOARD	?= pi3-arm64
#BOARD	?= pi-zero

# The directory where 'make install' puts the binary kernel.img
# This could be a mounted sd card, or the msd directory of rpi-boot
INSTALL_DIR	?= /data1/projects/raspberry-pi/usb-zero/msd

# Select your compiler here
ifeq ($(BOARD), pi3-arm64)

TGT_BOARD	?=	TGT_PI3_ARM64
#GNU_D	?=	/data1/gnu/gcc-linaro-6.3.1-2017.02-x86_64_aarch64-elf
GNU_D	?=	/data1/gnu/gcc-linaro-7.4.1-2019.02-x86_64_aarch64-elf

XGCC		?=	$(GNU_D)/bin/aarch64-elf-gcc
XLD			?=	$(GNU_D)/bin/aarch64-elf-ld
XOBJCOPY	?=	$(GNU_D)/bin/aarch64-elf-objcopy
LDLIB_D		?=	$(GNU_D)/aarch64-elf/libc/usr/lib/
LDSCRIPT	?=	$(DVSK_ROOT)/hardware/arm64/ld/dv-pi3.ldscript

ENTRY	?=	dv_reset

else

TGT_BOARD	?=	TGT_PI_ZERO
GNU_D	?=	/data1/gnu/gcc-linaro-6.3.1-2017.02-x86_64_arm-eabi

XGCC		?=	$(GNU_D)/bin/arm-eabi-gcc
XLD			?=	$(GNU_D)/bin/arm-eabi-ld
XOBJCOPY	?=	$(GNU_D)/bin/arm-eabi-objcopy
LDLIB_D		?=	$(GNU_D)/arm-eabi/libc/usr/lib/
LDSCRIPT	?=	$(DVSK_ROOT)/hardware/arm/ld/dv-pi-zero.ldscript

ENTRY	?=	dv_trap_reset

endif

BIN_D	= bin
OBJ_D	= obj

CC_OPT		+=	-D DV_DAVROSKA=1
CC_OPT		+=	-D TGT_BOARD=$(TGT_BOARD)
CC_OPT		+= -I h
CC_OPT		+= -I $(DVSK_ROOT)/h
CC_OPT		+= -I $(DVSK_ROOT)/hardware
CC_OPT		+= -I $(DV_ROOT)/devices/h
CC_OPT		+= -I $(DV_ROOT)/lib/h
CC_OPT		+= -Wall
CC_OPT		+= -fno-common

# -O3 doesn't work for some reason. The system doesn't start - or dv_printf() doesn't work.
CC_OPT		+= -O2

#CC_OPT		+= -mfloat-abi=hard
#CC_OPT		+= -mfpu=vfp

LD_OPT		+= -e $(ENTRY)
LD_OPT		+= -T $(LDSCRIPT)
LD_OPT		+=	-L $(LDLIB_D)
LD_OPT		+=	-lc

# The program code
LD_OBJS	+= $(OBJ_D)/jitter.o

# The frame manager
LD_OBJS	+= $(OBJ_D)/frame-manager.o

# davroska and associated library files
LD_OBJS	+= $(OBJ_D)/davroska.o
LD_OBJS	+= $(OBJ_D)/davroska-time.o
LD_OBJS	+= $(OBJ_D)/davroska-extended.o
LD_OBJS	+= $(OBJ_D)/dv-printf.o
LD_OBJS	+= $(OBJ_D)/dv-xprintf.o
LD_OBJS	+= $(OBJ_D)/dv-memset32.o

# Hardware-specific files
ifeq ($(BOARD), pi3-arm64)

LD_OBJS	+= $(OBJ_D)/davroska-arm64.o
LD_OBJS	+= $(OBJ_D)/jitter-pi3-arm64.o

LD_OBJS	+= $(OBJ_D)/dv-arm-bcm2835-uart.o
LD_OBJS	+= $(OBJ_D)/dv-arm-bcm2835-gpio.o
LD_OBJS	+= $(OBJ_D)/dv-arm-bcm2835-interruptcontroller.o
LD_OBJS	+= $(OBJ_D)/dv-arm-bcm2836-interruptcontroller.o

LD_OBJS	+= $(OBJ_D)/dv-armv8-mmu.o
LD_OBJS	+= $(OBJ_D)/dv-arm64-setmmuregisters.o
LD_OBJS	+= $(OBJ_D)/dv-arm64-reset.o
LD_OBJS	+= $(OBJ_D)/dv-switchcall.o
LD_OBJS	+= $(OBJ_D)/dv-switch-el3el2.o
LD_OBJS	+= $(OBJ_D)/dv-switch-el2el1.o
LD_OBJS	+= $(OBJ_D)/dv-vectors.o

else

LD_OBJS	+= $(OBJ_D)/davroska-arm.o
LD_OBJS	+= $(OBJ_D)/jitter-pi-zero.o

LD_OBJS	+= $(OBJ_D)/dv-arm-bcm2835-uart.o
LD_OBJS	+= $(OBJ_D)/dv-arm-bcm2835-gpio.o
LD_OBJS	+= $(OBJ_D)/dv-arm-bcm2835-interruptcontroller.o

LD_OBJS	+= $(OBJ_D)/dv-armv6-mmu.o
LD_OBJS	+= $(OBJ_D)/dv-vectors.o
LD_OBJS	+= $(OBJ_D)/dv-trap-irq.o
LD_OBJS	+= $(OBJ_D)/dv-switchcall.o
LD_OBJS	+= $(OBJ_D)/dv-arm-reset.o
LD_OBJS	+= $(OBJ_D)/dv-arm-traps.o

endif

VPATH 		+=	c
VPATH 		+=	s
VPATH		+=	$(DVSK_ROOT)/c
VPATH		+=	$(DVSK_ROOT)/c

ifeq ($(BOARD), pi3-arm64)

VPATH		+=	$(DVSK_ROOT)/hardware/arm64/c
VPATH		+=	$(DVSK_ROOT)/hardware/arm64/s

else

VPATH		+=	$(DVSK_ROOT)/hardware/arm/c
VPATH		+=	$(DVSK_ROOT)/hardware/arm/s

endif

VPATH		+=	$(DVSK_ROOT)/hardware/common/c
VPATH		+=	$(DV_ROOT)/lib/c
VPATH		+=	$(DV_ROOT)/devices/c
VPATH		+=	$(DV_ROOT)/devices/s


.PHONY:		default all help clean install srec

default:	all

clean:
	-rm -rf $(OBJ_D) $(BIN_D)

all:		$(OBJ_D) $(BIN_D) elf

elf:		$(BIN_D)/jitter.elf

$(BIN_D)/jitter.elf:	$(LD_OBJS)
	$(XLD) -o $@ $(LD_OBJS) $(LD_LIB) $(LD_OPT)

$(OBJ_D)/%.o:  %.c
	$(XGCC) $(CC_OPT) -o $@ -c $<

$(OBJ_D)/%.o:  %.S
	$(XGCC) $(CC_OPT) -o $@ -c $<

$(BIN_D):
	mkdir -p bin

$(OBJ_D):
	mkdir -p obj

install:	all
	$(XOBJCOPY) bin/jitter.elf -O binary $(INSTALL_DIR)/kernel.img

srec:		all
	$(XOBJCOPY) bin/jitter.elf -O srec --srec-forceS3 /dev/stdout | dos2unix | egrep -v '^S3..........00*..$$' > bin/jitter.srec
