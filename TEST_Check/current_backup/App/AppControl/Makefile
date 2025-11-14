include /project/system_mcts/current/App/AppControl/kjg_cflags.mk
CC	= gcc
INCLUDE	= -I. -I/usr/src/rtlinux-3.2-pre2/include
#CFLAGS	= -g -Wall -O2 -Wa,-a,-ad
CFLAGS	= -g -Wall -O2

.SUFFIXES :.o .c
.c.o:
	@echo "		Compiling......$<"
#	@$(CC) $(CFLAGS) -c $(INCLUDE) $<
	@$(CC) $(CFLAGS) $(KJG_CFLAGS) -c $(INCLUDE) $<

#OBJS = common_utils.o userlog.o \
	local_utils.o local_message.o StandardInput.o main.o

#kjhw_131030 common_message.o add  atype + btype
OBJS = common_utils.o common_message.o userlog.o \
	local_utils.o local_message.o StandardInput.o main.o

#Multi Channel Test System
mcts: $(OBJS)
	@echo "		Linking........$<"
	@$(CC) -lm -o $@ $(OBJS) $(LIBNAME)

#lint:
#	lint -I${INCLUDE} *.c

all_coa:
#	make clear_memory
	make clear_log
#	make clear_data
	make copy_common_program
	make rtl_resource
	make hw_test
	make source_program_coa
	(cd /; update -s1; sleep 3)

all_debug:
#	make clear_memory
	make clear_log
#	make clear_data
	make copy_common_program
	make rtl_resource
	make hw_test
	make source_program_coa
	(cd ../Sil_Client; make clean; make)
	(cd ../Sil_Client_2; make clean; make)
	(cd /; update -s1; sleep 3)
all_cob:
#	make clear_memory
	make clear_log
#	make clear_data
	make copy_common_program
	make rtl_resource
	make hw_test
	make source_program_cob
	(cd /; update -s1; sleep 3)

all_coc:
#	make clear_memory
	make clear_log
#	make clear_data
	make copy_common_program
	make rtl_resource
	make hw_test
	make source_program_coc
	(cd /; update -s1; sleep 3)

clear_memory:
	(cd /root/system_data/config/sharedMemory; rm -rf *; touch systemMemory)
	(cd /root/system_data/config/sharedMemory_dump; rm -rf *; touch systemMemory)

clear_log:
	(cd /root/system_data/log; rm -rf *)

clear_data:
	(cd /root/system_data; rm -rf resultData)
	(cd /root/system_data; rm -rf monitoringData)
	(cd /extended_partition; rm -rf *)
	(./dir_create)

copy_common_program:
	(cp -rf common_utils.* common_message.* userlog.* ../COA_Client)
	(cp -rf common_utils.* common_message.* userlog.* ../COB_Client)
	(cp -rf common_utils.* common_message.* userlog.* ../COC_Client)
	(cp -rf common_utils.* common_message.* userlog.* ../COD_Client)
	(cp -rf common_utils.* common_message.* userlog.* ../Jig_Control)
	(cp -rf common_utils.* common_message.* userlog.* ../DataSave)
	(cp -rf common_utils.* common_message.* userlog.* ../DumpMonitor)
	(cp -rf common_utils.* common_message.* userlog.* ../DebugMonitor)
	(cp -rf common_utils.* common_message.* userlog.* ../TimeSchedule) #kjh_160418
	(cp -rf common_utils.* common_message.* userlog.* ../Temp_Client)
	(cp -rf common_utils.* common_message.* userlog.* ../Can_Client)
	(cp -rf common_utils.* common_message.* userlog.* ../Sil_Client)
	(cp -rf common_utils.* common_message.* userlog.* ../Sil_Client_2)
	(cp -rf common_utils.* common_message.* ../../Module/mControl)

	(cd ../COA_Client; cp -rf com_io.* ../COB_Client)
	(cd ../COA_Client; cp -rf com_io.* ../COC_Client)
	(cd ../COA_Client; cp -rf com_io.* ../COD_Client)

	(cd ../COA_Client; cp -rf com_socket.* ../COB_Client)
	(cd ../COA_Client; cp -rf com_socket.* ../COC_Client)
	(cd ../COA_Client; cp -rf com_socket.* ../COD_Client)

rtl_resource:
	(cd ../../Module/mbuff; touch *.c; make)
	(cd ../../Module/rt_can; make clean; make; make install)
	(cd ../../Module/rt_com; make clean; make; make install)

hw_test:
	(cd ../../hwTest/hwTest_v1; make clean; make)
	(cd ../../hwTest/data_size_test; make clean; make)

source_program_coa:
	(touch *; make clean; make)
	(cd ../COA_Client; make clean; make)
#	(cd ../COB_Client; make clean; make)
#	(cd ../COC_Client; make clean; make)
#	(cd ../Jig_Control; make clean; make)
	(cd ../DataSave; make clean; make)
	(cd ../DumpMonitor; make clean; make)
	(cd ../DebugMonitor; make clean; make)
	(cd ../TimeSchedule; make clean; make)
	(cd ../Temp_Client; make clean; make)
	(cd ../Can_Client; make clean; make)
	(cd ../../Module/mControl; make clean; make)

source_program_cob:
	(touch *; make clean; make)
#	(cd ../COA_Client; make clean; make)
	(cd ../COB_Client; make clean; make)
#	(cd ../COC_Client; make clean; make)
	(cd ../Jig_Control; make clean; make)
	(cd ../DataSave; make clean; make)
	(cd ../DumpMonitor; make clean; make)
	(cd ../DebugMonitor; make clean; make)
	(cd ../../Module/mControl; make clean; make)

source_program_coc:
	(touch *; make clean; make)
#	(cd ../COA_Client; make clean; make)
#	(cd ../COB_Client; make clean; make)
	(cd ../COC_Client; make clean; make)
#	(cd ../Jig_Control; make clean; make)
	(cd ../DataSave; make clean; make)
	(cd ../DumpMonitor; make clean; make)
	(cd ../DebugMonitor; make clean; make)
	(cd ../../Module/mControl; make clean; make)

clear_program:
	rm -rf *.o; rm -rf *.c; rm -rf *.h
	(cd ../COA_Client; rm -rf *.o; rm -rf *.c; rm -rf *.h)
	(cd ../COB_Client; rm -rf *.o; rm -rf *.c; rm -rf *.h)
	(cd ../COC_Client; rm -rf *.o; rm -rf *.c; rm -rf *.h)
	(cd ../COD_Client; rm -rf *.o; rm -rf *.c; rm -rf *.h)
	(cd ../Jig_Control; rm -rf *.o; rm -rf *.c; rm -rf *.h)
	(cd ../DataSave; rm -rf *.o; rm -rf *.c; rm -rf *.h)
#	(cd ../DumpMonitor; rm -rf *.o; rm -rf *.c; rm -rf *.h)
#	(cd ../DebugMonitor; rm -rf *.o; rm -rf *.c; rm -rf *.h)
	(cd ../Can_Client; rm -rf *.o; rm -rf *.c; rm -rf *.h)
	(cd ../Sil_Client; make clean; make)
	(cd ../Sil_Client_2; make clean; make)
	(cd ../../Module/mControl; rm -rf *.o; rm -rf *.c; rm -rf *.h)
#	(cd ../../INC; rm -rf *)

clean:
	rm -rf *.o
