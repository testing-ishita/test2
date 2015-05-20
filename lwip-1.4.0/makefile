################################################################################
# 
# Filename:	 makefile
# 
# Description:	lwIP top level makefile, modified for CCT CUTE. 
# 
# $Revision: 1.2 $
# 
# $Date: 2013-09-26 12:20:10 $
# 
# $Source: /home/cvs/cvsroot/CCT_BIT_2/lwip-1.4.0/makefile,v $
#
# Copyright 2012-2013 Concurrent Technologies, Plc.
#
################################################################################

WARN	=

AS 	= as
CC	= gcc
LD	= ld

DESTDIR = ../board/$(BOARD)/bin
LWIPDIR = .
LWIPSRCDIR = $(LWIPDIR)/src
LWIPINCDIR = $(LWIPSRCDIR)/include

INCLUDE	= -I$(LWIPINCDIR) -I$(LWIPINCDIR)/ipv4 -I$(LWIPDIR)/ports/include -I$(LWIPDIR)/lwIP_Apps
BOARDINC = -I../board/$(BOARD)
FREERTOSINC= -I../FreeRTOS/Demo/Common/include -I../FreeRTOS/Source/include -I../FreeRTOS/Demo/CUTE -I../include

AFLAGS	= -I$(INCLUDE) --fatal-warnings
CFLAGS	= -c $(INCLUDE) $(FREERTOSINC) $(BOARDINC) -DIPv4 -DGCC_CUTE -Wall $(WARN)
# -DLWIP_DEBUG
# -Wa,-alsn=$@.lst
LDFLAGS = -s

# COREFILES, CORE4FILES: The minimum set of files needed for lwIP.
COREFILES=$(LWIPSRCDIR)/core/mem.c $(LWIPSRCDIR)/core/memp.c $(LWIPSRCDIR)/core/netif.c \
	$(LWIPSRCDIR)/core/pbuf.c $(LWIPSRCDIR)/core/stats.c $(LWIPSRCDIR)/core/sys.c \
	$(LWIPSRCDIR)/core/tcp.c $(LWIPSRCDIR)/core/tcp_in.c $(LWIPSRCDIR)/core/raw.c \
	$(LWIPSRCDIR)/core/tcp_out.c $(LWIPSRCDIR)/core/udp.c $(LWIPSRCDIR)/core/init.c \
	$(LWIPSRCDIR)/core/def.c $(LWIPSRCDIR)/core/timers.c $(LWIPSRCDIR)/core/dhcp.c
        
CORE4FILES=$(LWIPSRCDIR)/core/ipv4/icmp.c $(LWIPSRCDIR)/core/ipv4/ip.c \
	$(LWIPSRCDIR)/core/ipv4/inet.c $(LWIPSRCDIR)/core/ipv4/ip_addr.c \
	$(LWIPSRCDIR)/core/ipv4/inet_chksum.c $(LWIPSRCDIR)/core/ipv4/ip_frag.c
	
# APIFILES: The files which implement the sequential and socket APIs.
APIFILES=$(LWIPSRCDIR)/api/api_lib.c $(LWIPSRCDIR)/api/api_msg.c $(LWIPSRCDIR)/api/tcpip.c \
	$(LWIPSRCDIR)/api/err.c $(LWIPSRCDIR)/api/sockets.c $(LWIPSRCDIR)/api/netbuf.c $(LWIPSRCDIR)/api/netdb.c

# NETIFFILES: Files implementing various generic network interface functions.
NETIFFILES=$(LWIPSRCDIR)/netif/etharp.c
	
# ARCHFILES: Architecture specific files.
ARCHFILES=$(LWIPDIR)/ports/sys_arch.c $(LWIPDIR)/ports/netif/gei_netif.c

# APPFILES: Application specific files.
APPFILES=$(LWIPDIR)/lwIP_Apps/lwIP_Apps.c
 
CMDSVRFILES=$(LWIPDIR)/lwIP_Apps/apps/CommandServer/CommandServer.c \
	$(LWIPDIR)/lwIP_Apps/utils/CommandInterpreter.c
	
PRDCSMFILES=$(LWIPDIR)/lwIP_Apps/apps/ProducerConsumer/tcpCliSvr.c

SMFILES=$(LWIPDIR)/lwIP_Apps/apps/SmClient/smClient.c
	
# LWIPFILES: All the above.
LWIPFILES=$(COREFILES) $(CORE4FILES) $(APIFILES) $(NETIFFILES) $(ARCHFILES) $(SMFILES) $(APPFILES) $(CMDSVRFILES) $(PRDCSMFILES)
LWIPFILESW=$(wildcard $(LWIPFILES))
LWIPOBJS=$(notdir $(LWIPFILESW:.c=.o))

LWIPLIB = liblwip.a

#all: $(LWIPLIB) 	
all: $(DESTDIR)/$(LWIPLIB)
.PHONY: all

include .depend

%.o:
	$(CC) $(CFLAGS) $(<:.o=.c)

$(DESTDIR)/$(LWIPLIB):  $(LWIPLIB)
	cp $(LWIPLIB) $(DESTDIR)

$(LWIPLIB): $(LWIPOBJS)
	$(AR) -rcs $(LWIPLIB) $(LWIPOBJS)
	$(AR) -tv $(LWIPLIB)

.depend: $(LWIPFILES)
	$(CC) $(CFLAGS) -MM $^ > .depend
	
clean:
	rm -f *.o
	rm -f *.o.lst
	rm -f *.a
	rm -f .depend
	
