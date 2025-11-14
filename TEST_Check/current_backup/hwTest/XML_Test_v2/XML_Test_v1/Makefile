CC	= gcc
CFLAGS	= -g -Wall
INCLUDE	= -I. -I/usr/include/gnome-xml

.SUFFIXES :.o .c
.c.o:
	@echo "		Comiling...........$<"
	@$(CC) $(CFLAGS) -O2 -c $(INCLUDE) $<

OBJS =  main.o

xmlTest_v1: $(OBJS)
	@echo "		Linking............$<"
	@$(CC) -lxml -lz -o $@ $(OBJS)

clean:
	rm -rf *.o *.csv
