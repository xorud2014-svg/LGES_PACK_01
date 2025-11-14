# Standalone Makefile for rt_com

SYSTEM			= RTLINUX_V3

INCLUDEDIR		= /usr/src/rtlinux-3.2-pre2/include

INSTALL 		= install -c -m 644

OBJS			= rt_com.o

VERSION			= kjg1

#include rtl.mk
include /project/system_mcts/current/Module/mControl/rtl.mk
include /project/system_mcts/current/App/AppControl/kjg_cflags.mk

CFLAGS			+= -D$(SYSTEM) -DVERSION=\"$(VERSION)\"

.c.o:	Makefile
		@echo "		Compiling......$<"
#		@$(CC) $(CFLAGS) -c $<
		@$(CC) $(CFLAGS) $(KJG_CFLAGS) -c $<

all:			$(OBJS)

clean:
			rm -fr $(OBJS) rt_com-$(VERSION)
			rm -f *.o

install:	rt_com.o
			$(INSTALL) $(OBJS) /lib/modules/`uname -r`/misc/
			$(INSTALL) rt_com.h $(INCLUDEDIR)/rt_com.h

