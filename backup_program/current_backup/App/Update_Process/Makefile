include /project/system_mcts/current/App/AppControl/kjg_cflags.mk
CC = gcc
INCLUDE = -I. -I/usr/src/rtlinux-3.2-pre2/include
CFLAGS = -g -Wall -O2

.SUFFIXES :.o .c
.c.o:
	@echo "		Compiling......$<"
#	@$(CC) $(CFLAGS) -c $(INCLUDE) $<
	@$(CC) $(CFLAGS) $(KJG_CFLAGS) -c $(INCLUDE) $<
	
OBJS = common_utils.o common_message.o userlog.o \
	local_utils.o local_message.o main.o

Update_Process: $(OBJS)
	@echo "		Linking........$<"
	@$(CC) -lm -o $@ $(OBJS) $(LIBNAME)

clean:
	rm -rf *.o
