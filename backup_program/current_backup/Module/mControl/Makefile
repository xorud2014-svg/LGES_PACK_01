include rtl.mk
include /project/system_mcts/current/App/AppControl/kjg_cflags.mk

.SUFFIXES :.o .c
.c.o:
	@echo "		Compiling......$<"
#	@$(CC) $(CFLAGS) -c $(INCLUDE) $<
	@$(CC) $(CFLAGS) $(KJG_CFLAGS) -c $(INCLUDE) $<

#OBJ	= rt_can.o common_utils.o common_message.o
ifeq ($(findstring CAN_FD, $(KJG_CFLAGS)), CAN_FD)
OBJ	= rt_can.o common_utils.o common_message.o\
	local_utils.o local_message.o ModuleControl.o GroupControl.o\
	BoardControl.o\
	ch_utils.o ChannelControl.o Analog.o FCH_Control.o PWM3_Control.o\
	InOutControl.o SubSensV.o CAN.o COM.o rtTask.o
else
OBJ	= common_utils.o common_message.o\
	local_utils.o local_message.o ModuleControl.o GroupControl.o\
	BoardControl.o\
	ch_utils.o ChannelControl.o Analog.o FCH_Control.o PWM3_Control.o\
	InOutControl.o SubSensV.o CAN.o COM.o rtTask.o
endif

mControl: $(OBJ) $(HEADER)
	@echo "		Linking........$<"
	@$(LD) $(LDFLAGS) -o $@ $(OBJ)

unload:
	@echo "		remove real time task......"
	(./rmrtl)

load:
	@make unload
	@echo "		Before load  real time task......"
	(./insrtl)

clean:
	rm -rf *.o
