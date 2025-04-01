include /project/system_mcts/current/App/AppControl/kjg_cflags.mk
CC = gcc
INCLUDE	= -I. -I/usr/src/rtlinux-3.2-pre2/include
CFLAGS = -g -Wall -O2

.SUFFIXES :.o .c
.c.o:
	@echo "		Compiling......$<"
#	@$(CC) $(CFLAGS) -c $(INCLUDE) $<
	@$(CC) $(CFLAGS) $(KJG_CFLAGS) -c $(INCLUDE) $<

OBJS = common_utils.o common_message.o userlog.o \
	local_utils.o local_message.o com_io.o com_socket.o network.o main.o

TempClient: $(OBJS)
	@echo "		Linking........$<"
	@$(CC) -lm -o $@ $(OBJS) $(LIBNAME)
	@ln -s -f TempClient Temp1Client
	@ln -s -f TempClient Temp2Client
	@ln -s -f TempClient Temp3Client
	@ln -s -f TempClient Temp4Client

clean:
	rm -rf *.o
	rm -rf Temp*
