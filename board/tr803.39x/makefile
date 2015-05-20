
# ************************************************************************
# *                                                                      *
# *      Copyright 2009 Concurrent Technologies, all rights reserved.    *
# *                                                                      *
# *      The program below is supplied by Concurrent Technologies        *
# *      on the understanding that no responsibility is assumed by       *
# *      Concurrent Technologies for any errors contained therein.       *
# *      Furthermore, Concurrent Technologies makes no commitment to     *
# *      update or keep current the program code, and reserves the       *
# *      right to change its specifications at any time.                 *
# *                                                                      *
# *      Concurrent Technologies assumes no responsibility either for    *
# *      the use of this code or for any infringements of the patent     *
# *      or other rights of third parties which may result from its use  *
# *                                                                      *
# ************************************************************************

# MAKEFILE - Board-specific build
#


WARN	=
BITFLAG =
RELARGS =
STARTUPFLAG =
BOARD       =
IMAGETYPE = COMPRESSED
SYMBOLS = NO


AS	= as
CC	= gcc
LD	= ld

TOOLS	= ../../tools.cct
INCLUDE	= ../../include
BOARDINC = ../../board/$(BOARD)

LWIPDIR = ../../lwip-1.4.0
LWIPINC = -I$(LWIPDIR)/src/include -I$(LWIPDIR)/src/include/ipv4 -I$(LWIPDIR)/ports/include -I$(LWIPDIR)/lwIP_Apps


AFLAGS	= -I$(INCLUDE) --fatal-warnings
CFLAGS	= -c -I$(INCLUDE) -I$(BOARDINC) -Wall $(WARN)

ifeq ($(SYMBOLS), YES)
LDFLAGS = --library-path="./bin"
else
LDFLAGS = --library-path="./bin" -s
endif


ifeq ($(BITFLAG), MAKE_BIT)
OBJS	= board_cfg.o hw_init.o nvram.o pci_cfg.o signon.o test_list.o	\
	      watchdog.o superio.o pci.o postLED.o SCR.o eeprom.o 		    \
	      DispConfig.o cpu.o smbus.o LEDs.o app_flash.o temperature.o   \
	      board_delay.o serialbaud.o spieeprom.o ldtpit.o gpio.o        \
	      board_service.o spimram.o network.o
endif
	      
ifeq ($(BITFLAG), MAKE_CUTE)
OBJS	= board_cfg.o hw_init.o nvram.o pci_cfg.o signon.o test_list.o	\
	      watchdog.o superio.o pci.o postLED.o SCR.o eeprom.o 		    \
	      DispConfig.o cpu.o smbus.o LEDs.o app_flash.o temperature.o   \
	      board_delay.o interconnect.o serialbaud.o spieeprom.o gpio.o  \
	      board_service.o ldtpit.o spimram.o idtTempSensor.o network.o
endif

ifeq ($(BITFLAG), MAKE_BIT_CUTE)
OBJS	= board_cfg.o hw_init.o nvram.o pci_cfg.o signon.o test_list.o	\
	      watchdog.o superio.o pci.o postLED.o SCR.o eeprom.o 		    \
	      DispConfig.o cpu.o smbus.o LEDs.o app_flash.o temperature.o   \
	      board_delay.o interconnect.o serialbaud.o spieeprom.o gpio.o  \
	      board_service.o ldtpit.o spimram.o idtTempSensor.o network.o
endif
	      
ifeq ($(BITFLAG), MAKE_BIT)
LIBS    = -lbittest -lblib -lclib -lconslib -lfreertos \
          -lkernel -libit -lpbit
MODULES	= ../../loader/loader.o ../../kernel/libkernel.a ../../blib/libblib.a \
		  ../../clib/libclib.a ../../ibit/libibit.a ../../pbit/libpbit.a      \
		  ../../tests/libbittest.a ../../conslib/libconslib.a board.o         \
		  ../../FreeRTOS/Demo/CUTE/libfreertos.a  ../../debug/libdebug.a
		  
ALLFILES = ../../loader/* ../../kernel/* ../../blib/* ../../clib/* \
           ../../ibit/* ../../pbit/*  ../../conslib/* ./*  ../../debug/*


endif

ifeq ($(BITFLAG), MAKE_CUTE)
LIBS    = -lbittest -lblib -lclib -lconslib -lfreertos \
          -lkernel -lmth -lsth -lvpxArch -lsm3 -lserialComm \
          -linterconnect
  	  
MODULES	= ../../loader/loader.o ../../kernel/libkernel.a ../../blib/libblib.a              \
		  ../../clib/libclib.a ../../sth/libsth.a                       \
		  ../../tests/libbittest.a ../../mth/libmth.a ../../conslib/libconslib.a board.o   \
		  ../../arch/vpx/libvpxArch.a  ../../xlib/sm3/libsm3.a                           \
		  ../../FreeRTOS/Demo/CUTE/libfreertos.a ../../comm/serial/libserialComm.a         \
		  ../../xlib/interconnect/libinterconnect.a ../../vgalib/libvga.a ../../debug/libdebug.a \
		  ../../lwip-1.4.0/liblwip.a

ALLFILES = ../../loader/* ../../kernel/* ../../blib/* ../../clib/*  \
           ../../mth/* ../../sth/* ../../conslib/* ../../arch/vpx/* \
           ../../xlib/sm3/* ./* ../../comm/serial/*                 \
           ../../xlib/interconnect/* ../../vgalib/* ../../debug/*

endif
		  
		  
ifeq ($(BITFLAG), MAKE_BIT_CUTE)
LIBS    = -lbittest -lblib -lclib -lconslib -lfreertos       \
          -lkernel -libit -lpbit -lmth -lsth -lvpxArch -lsm3 \
          -lserialComm -linterconnect
                                 		  
MODULES	= ../../loader/loader.o ../../kernel/libkernel.a ../../blib/libblib.a            \
		  ../../clib/libclib.a ../../ibit/libibit.a ../../pbit/libpbit.a                 \
		  ../../tests/libbittest.a ../../mth/libmth.a ../../sth/libsth.a board.o         \
		  ../../conslib/libconslib.a ../../arch/vpx/libvpxArch.a ../../xlib/sm3/libsm3.a \
		   ../../FreeRTOS/Demo/CUTE/libfreertos.a ../../comm/serial/libserialComm.a      \
		   ../../xlib/interconnect/libinterconnect.a
		  
ALLFILES = ../../loader/* ../../kernel/* ../../blib/* ../../clib/*           \
           ../../ibit/* ../../pbit/* ../../mth/* ../../sth/* ../../conslib/* \
           ../../arch/vpx/* ../../xlib/sm3/*  ../../comm/serial/*            \
           ../../xlib/interconnect/*

endif


TARGET	= ./bin/cctbit.bin

# Primary Target
# --------------

# Note: version must be first, otherwise signon maybe out of date!
board.o: version $(OBJS)
	$(LD) -r -o board.o $(OBJS)


ifeq ($(IMAGETYPE), COMPRESSED)

all: package.bin ../../loader/boot_real.bin compress.bin
	cat ../../loader/boot_real.bin ./bin/compress.bin > $(TARGET)
	$(TOOLS)/bit_tool $(TARGET)

compress.bin: ../../loader/comp_loader.o complinkfile
	$(TOOLS)/bfc c lz ./bin/package.bin ./bin/compress.lz
	objcopy --input-target binary --output-target elf32-i386 \
		--binary-architecture i386 ./bin/compress.lz ./bin/compress.o
	$(LD) -s -Tcomplinkfile -e _comp_entry -o ./bin/compress.out \
	../../loader/comp_loader.o ./bin/compress.o -Map ./bin/compress.map
	objcopy -O binary ./bin/compress.out ./bin/compress.bin

else

all: package.bin ../../loader/boot_real.bin
	cat ../../loader/boot_real.bin ./bin/package.bin > $(TARGET)
	$(TOOLS)/bit_tool $(TARGET)

endif

ifeq ($(SYMBOLS), YES)
# Create a symbol table, compile and link into the build
package.bin: $(MODULES) linkfile $(ALLFILES)
	$(CC) -r -nostdlib -Wl,-X -Wl,--library-path="./bin" -Wl,$(MODULES) \
	  -Wl,--start-group $(LIBS) -Wl,--end-group -o ./bin/tmp.out
	nm -C -n ./bin/tmp.out > ./bin/partial.txt
	rm -f ./bin/tmp.out
	rm -f ./bin/symtbl.*
	$(TOOLS)/symtbl ./bin/partial.txt ./bin/symtbl.c
	$(CC) -c -w -I$(INCLUDE) ./bin/symtbl.c -o ./bin/symtbl.o
	$(LD) $(LDFLAGS) -Tlinkfile -e _prot_entry -o ./bin/package.out $(MODULES) \
	  ./bin/symtbl.o $(LIBS) -Map ./bin/cctbit.map
	nm -C -n ./bin/package.out > ./bin/symbols.txt
	$(TOOLS)/ariumsym -i ./bin/symbols.txt -o ./bin/symbols.sym
	objcopy -O binary ./bin/package.out ./bin/package.bin
else
package.bin: $(MODULES) linkfile $(ALLFILES)
	$(LD) $(LDFLAGS) -Tlinkfile -e _prot_entry -o ./bin/package.out $(MODULES) \
	$(LIBS) -Map ./bin/cctbit.map
	objcopy -O binary ./bin/package.out ./bin/package.bin
endif

# Secondary Targets

version :
	$(TOOLS)/BIT_VER TR803_39X $(BITFLAG) $(RELARGS)

board_delay.o: board_delay.S \
	$(INCLUDE)/asm/i386.h
	$(AS) $(AFLAGS) board_delay.S -o board_delay.o

board_cfg.o: board_cfg.c						         \
	$(INCLUDE)/stdtypes.h $(INCLUDE)/bit/board_service.h \
	$(INCLUDE)/private/port_io.h                         \
    $(INCLUDE)/private/sys_delay.h
	$(CC) $(CFLAGS) board_cfg.c

board_service.o: board_service.c						 \
	$(INCLUDE)/stdtypes.h $(INCLUDE)/bit/board_service.h \
	$(INCLUDE)/private/sys_delay.h
ifeq ($(BITFLAG), MAKE_BIT)
	$(CC) $(CFLAGS) board_service.c
else
	$(CC) $(CFLAGS) -DCUTE board_service.c
endif


hw_init.o: hw_init.c							\
	$(INCLUDE)/stdtypes.h						\
	$(INCLUDE)/private/port_io.h 				\
	$(BOARDINC)/config.h
	$(CC) $(CFLAGS) hw_init.c

nvram.o: nvram.c								\
	$(INCLUDE)/stdtypes.h						\
	$(INCLUDE)/private/port_io.h
	$(CC) $(CFLAGS) nvram.c

pci_cfg.o: pci_cfg.c							\
	$(INCLUDE)/stdtypes.h 						\
	$(INCLUDE)/bit/pci.h						\
	$(INCLUDE)/private/port_io.h
	$(CC) $(CFLAGS) pci_cfg.c

signon.o: signon.c							    \
	version.h							        \
	$(INCLUDE)/stdtypes.h $(INCLUDE)/stdio.h    \
	$(INCLUDE)/bit/board_service.h
	$(CC) $(CFLAGS) signon.c

test_list.o: test_list.c						\
	$(INCLUDE)/stdtypes.h 						\
	$(INCLUDE)/bit/bit.h						\
	$(INCLUDE)/bit/tests.h
ifeq ($(BITFLAG), MAKE_BIT)
	$(CC) $(CFLAGS) test_list.c
else
	$(CC) $(CFLAGS) -DCUTE test_list.c
endif

watchdog.o: watchdog.c							\
	$(INCLUDE)/stdtypes.h						\
	$(INCLUDE)/private/port_io.h
	$(CC) $(CFLAGS) watchdog.c

superio.o: superio.c							\
	$(INCLUDE)/stdtypes.h						\
	$(INCLUDE)/bit/bit.h						\
	$(INCLUDE)/private/port_io.h
	$(CC) $(CFLAGS) superio.c

pci.o: pci.c								    \
	$(INCLUDE)/stdtypes.h						\
	$(INCLUDE)/bit/bit.h						\
	$(INCLUDE)/private/port_io.h
	$(CC) $(CFLAGS) pci.c


postLED.o: postLED.c							\
	$(INCLUDE)/stdtypes.h						\
	$(INCLUDE)/bit/bit.h						\
	$(INCLUDE)/private/port_io.h
	$(CC) $(CFLAGS) postLED.c

gpio.o: gpio.c						        \
	$(INCLUDE)/stdtypes.h $(INCLUDE)/bit/board_service.h		\
	$(INCLUDE)/private/sys_delay.h
	$(CC) $(CFLAGS) gpio.c

idtTempSensor.o: idtTempSensor.c						        \
	$(INCLUDE)/stdtypes.h $(INCLUDE)/bit/board_service.h		\
	$(INCLUDE)/private/sys_delay.h
	$(CC) $(CFLAGS) idtTempSensor.c

SCR.o: SCR.c								\
	$(INCLUDE)/stdtypes.h						\
	$(INCLUDE)/bit/bit.h						\
	$(INCLUDE)/private/port_io.h
	$(CC) $(CFLAGS) SCR.c

eeprom.o: eeprom.c							\
	$(INCLUDE)/stdtypes.h						\
	$(INCLUDE)/bit/bit.h						\
	$(INCLUDE)/private/port_io.h
	$(CC) $(CFLAGS) eeprom.c

DispConfig.o: DispConfig.c						\
	$(INCLUDE)/stdtypes.h						\
	$(INCLUDE)/bit/bit.h						\
	$(INCLUDE)/private/port_io.h
	$(CC) $(CFLAGS) DispConfig.c

cpu.o: cpu.c								\
	$(INCLUDE)/stdtypes.h
	$(CC) $(CFLAGS) cpu.c

smbus.o: smbus.c							\
	$(INCLUDE)/stdtypes.h						\
	$(INCLUDE)/bit/bit.h						\
	$(INCLUDE)/private/port_io.h
	$(CC) $(CFLAGS) smbus.c

LEDs.o: LEDs.c								\
	$(INCLUDE)/stdtypes.h						\
	$(INCLUDE)/bit/bit.h						\
	$(INCLUDE)/private/port_io.h
	$(CC) $(CFLAGS) LEDs.c

ldtpit.o: ldtpit.c								\
	$(INCLUDE)/stdtypes.h						\
	$(INCLUDE)/bit/bit.h						\
	$(INCLUDE)/private/port_io.h
	$(CC) $(CFLAGS) ldtpit.c

app_flash.o: app_flash.c							\
	$(INCLUDE)/stdtypes.h						\
	$(INCLUDE)/bit/bit.h $(INCLUDE)/bit/board_service.h			\
	$(INCLUDE)/devices/mirrorflash.h
	$(CC) $(CFLAGS) app_flash.c

temperature.o: temperature.c							\
	$(INCLUDE)/stdtypes.h						\
	$(INCLUDE)/bit/bit.h $(INCLUDE)/bit/board_service.h
	$(CC) $(CFLAGS) temperature.c

serialbaud.o: serialbaud.c							\
	$(INCLUDE)/stdtypes.h						\
	$(INCLUDE)/bit/bit.h						\
	$(INCLUDE)/private/port_io.h $(INCLUDE)/bit/board_service.h
	$(CC) $(CFLAGS) serialbaud.c

spieeprom.o: spieeprom.c							\
	$(INCLUDE)/stdtypes.h						\
	$(INCLUDE)/bit/bit.h						\
	$(INCLUDE)/private/port_io.h $(INCLUDE)/bit/board_service.h
	$(CC) $(CFLAGS) spieeprom.c

spimram.o: spimram.c							\
	$(INCLUDE)/stdtypes.h						\
	$(INCLUDE)/bit/bit.h						\
	$(INCLUDE)/private/port_io.h $(INCLUDE)/bit/board_service.h
	$(CC) $(CFLAGS) spimram.c

network.o: network.c		\
	$(INCLUDE)/stdtypes.h 	\
	config.h
ifeq ($(BITFLAG), MAKE_BIT)
	$(CC) $(CFLAGS) network.c
else
	$(CC) $(CFLAGS) $(LWIPINC) -DCUTE network.c
endif

ifeq ($(BITFLAG), MAKE_BIT)
../../ibit/libibit.a: FORCE
	(cd ../..; cd ibit; make all WARN=$(WARN) BITFLAG=$(BITFLAG) BOARD=$(BOARD))

../../pbit/libpbit.a: FORCE
	(cd ../..; cd pbit; make all WARN=$(WARN) BITFLAG=$(BITFLAG) BOARD=$(BOARD))

../../kernel/libkernel.a: FORCE
	(cd ../..; cd kernel; make WARN=$(WARN) BITFLAG=$(BITFLAG) STARTUPFLAG=$(STARTUPFLAG)\
	                                        BOARD=$(BOARD)  BTYPE=VPX)

../../debug/libdebug.a: FORCE
	(cd ../..; cd debug; make all WARN=$(WARN) BITFLAG=$(BITFLAG) BOARD=$(BOARD))

endif


ifeq ($(BITFLAG), MAKE_CUTE)

interconnect.o: interconnect.c						\
	$(INCLUDE)/stdtypes.h $(INCLUDE)/stdio.h			\
	$(INCLUDE)/errors.h						\
	$(INCLUDE)/bit/bit.h $(INCLUDE)/bit/board_service.h \
	$(INCLUDE)/private/semaphore.h
	$(CC) $(CFLAGS) interconnect.c -DCUTE

../../mth/libmth.a: FORCE
	(cd ../..; cd mth; make all WARN=$(WARN) BITFLAG=$(BITFLAG) BOARD=$(BOARD))

../../sth/libsth.a: FORCE
	(cd ../..; cd sth; make all WARN=$(WARN) BITFLAG=$(BITFLAG) BTYPE=VPX BOARD=$(BOARD))

../../arch/vpx/libvpxArch.a: FORCE
	(cd ../..; cd arch; cd vpx; make all WARN=$(WARN) BITFLAG=$(BITFLAG) BOARD=$(BOARD))

../../xlib/sm3/libsm3.a: FORCE
	(cd ../..; cd xlib; cd sm3; make all WARN=$(WARN) BITFLAG=$(BITFLAG) BOARD=$(BOARD))

../../xlib/interconnect/libinterconnect.a: FORCE
	(cd ../..; cd xlib; cd interconnect; make all WARN=$(WARN) BITFLAG=$(BITFLAG) BOARD=$(BOARD)\
	                                               BTYPE=VPX )

../../comm/serial/libserialComm.a: FORCE
	(cd ../..; cd comm; cd serial; make all WARN=$(WARN) BITFLAG=$(BITFLAG) BOARD=$(BOARD))

../../kernel/libkernel.a: FORCE
	(cd ../..; cd kernel; make WARN=$(WARN) BITFLAG=$(BITFLAG) STARTUPFLAG=$(STARTUPFLAG)\
	                      BOARD=$(BOARD) BTYPE=VPX)

../../vgalib/libvga.a: FORCE
	(cd ../..; cd vgalib; make all WARN=$(WARN) BITFLAG=$(BITFLAG) BOARD=$(BOARD))

../../debug/libdebug.a: FORCE
	(cd ../..; cd debug; make all WARN=$(WARN) BITFLAG=$(BITFLAG) BOARD=$(BOARD))

../../lwip-1.4.0/liblwip.a: FORCE
	(cd ../..; cd lwip-1.4.0; make all WARN=$(WARN) BITFLAG=$(BITFLAG) BOARD=$(BOARD))

endif


ifeq ($(BITFLAG), MAKE_BIT_CUTE)

interconnect.o: interconnect.c						\
	$(INCLUDE)/stdtypes.h $(INCLUDE)/stdio.h		\
	$(INCLUDE)/errors.h								\
	$(INCLUDE)/bit/bit.h $(INCLUDE)/bit/board_service.h 	\
	$(INCLUDE)/private/semaphore.h
	$(CC) $(CFLAGS) interconnect.c -DCUTE


../../ibit/libibit.a: FORCE
	(cd ../..; cd ibit; make all WARN=$(WARN) BITFLAG=$(BITFLAG) BOARD=$(BOARD))

../../pbit/libpbit.a: FORCE
	(cd ../..; cd pbit; make all WARN=$(WARN) BITFLAG=$(BITFLAG) BOARD=$(BOARD))

../../mth/libmth.a: FORCE
	(cd ../..; cd mth; make all WARN=$(WARN) BITFLAG=$(BITFLAG) BOARD=$(BOARD))

../../sth/libsth.a: FORCE
	(cd ../..; cd sth; make all WARN=$(WARN) BITFLAG=$(BITFLAG)  BTYPE=VPX BOARD=$(BOARD))

../../arch/vpx/libvpxArch.a: FORCE
	(cd ../..; cd arch; cd vpx; make all WARN=$(WARN) BITFLAG=$(BITFLAG) BOARD=$(BOARD))	
	
../../xlib/interconnect/libinterconnect.a: FORCE
	(cd ../..; cd xlib; cd interconnect; make all WARN=$(WARN) BITFLAG=$(BITFLAG) BOARD=$(BOARD) \
	                                               BTYPE=VPX )

../../comm/serial/libserialComm.a: FORCE
	(cd ../..; cd comm; cd serial; make all WARN=$(WARN) BITFLAG=$(BITFLAG) BOARD=$(BOARD))

../../xlib/sm3/libsm3.a: FORCE
	(cd ../..; cd xlib; cd sm3; make all WARN=$(WARN) BITFLAG=$(BITFLAG) BOARD=$(BOARD))

../../kernel/libkernel.a: FORCE
	(cd ../..; cd kernel; make WARN=$(WARN) BITFLAG=$(BITFLAG) STARTUPFLAG=$(STARTUPFLAG) \
	                      BOARD=$(BOARD) BTYPE=VPX)

endif


../../conslib/libconslib.a: FORCE
	(cd ../..; cd conslib; make all WARN=$(WARN) BITFLAG=$(BITFLAG) BOARD=$(BOARD))

../../blib/libblib.a: FORCE
	(cd ../..; cd blib; make all WARN=$(WARN) BITFLAG=$(BITFLAG) BOARD=$(BOARD))

../../clib/libclib.a: FORCE
	(cd ../..; cd clib; make all WARN=$(WARN) BITFLAG=$(BITFLAG) BOARD=$(BOARD))

../../loader/boot_real.bin:
	(cd ../..; cd loader; make all WARN=$(WARN) BITFLAG=$(BITFLAG) IMAGETYPE=$(IMAGETYPE))

../../loader/loader.o: FORCE
	(cd ../..; cd loader; make all WARN=$(WARN) BITFLAG=$(BITFLAG) IMAGETYPE=$(IMAGETYPE))

../../tests/libbittest.a: FORCE
	(cd ../..; cd tests; make all WARN=$(WARN) BITFLAG=$(BITFLAG) \
		BTYPE=VPX BOARD=$(BOARD))

../../FreeRTOS/Demo/CUTE/libfreertos.a: FORCE
	(cd ../..; cd FreeRTOS/Demo/CUTE; make all BOARD=$(BOARD) SYMBOLS=$(SYMBOLS))

FORCE:


# Minor Targets

clean:
	rm -f *.o
	rm -f ./bin/*.out
	rm -f ./bin/*.bin
	rm -f ./bin/*.map
	rm -f ./bin/*.a
	rm -f ./bin/*.o
	rm -f ./bin/*.lz
	rm -f ./bin/symbols.txt
	rm -f ./bin/symbols.sym
	rm -f ./bin/partial.txt
	rm -f ./bin/symtbl.*

