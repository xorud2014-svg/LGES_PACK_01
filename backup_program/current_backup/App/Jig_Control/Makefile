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
	local_utils.o local_message.o \
	jig_type.o tray_type.o door_type.o stacker_type.o stopper_type.o\
	lamp_type.o emg_type.o job_change.o motor_control.o main.o

Jig_Control: $(OBJS)
	@echo "		Linking........$<"
	@$(CC) -lm -o $@ $(OBJS) $(LIBNAME)
	@ln -s -f Jig_Control Jig1_Control
	@ln -s -f Jig_Control Jig2_Control

clean:
	rm -rf *.o
