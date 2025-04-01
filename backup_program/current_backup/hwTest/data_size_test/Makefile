include /project/system_mcts/current/App/AppControl/kjg_cflags.mk
CC	= gcc
CFLAGS	= -g
INCLUDE	= -I/usr/src/linux/include

.SUFFIXES :.o .c
.c.o:
	@echo "		Comiling..............................$<"
	@$(CC) $(CFLAGS) $(KJG_CFLAGS) -O2 -c $(INCLUDE) $<

OBJ =  main.o

all:	data_size_test

data_size_test: $(OBJ)
	@echo "		Linking...............................$<"
	@$(CC) -o $@ $(OBJ)
clean:
	rm -rf *.o data_size_test
