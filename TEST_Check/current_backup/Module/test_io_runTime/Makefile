include rtl.mk

.SUFFIXES :.o .c
.c.o:
	@echo "		Compiling......$<"
	@$(CC) $(CFLAGS) -c $(INCLUDE) $<

OBJ	= rtTask.o

tControl: $(OBJ) $(HEADER)
	@echo "		Linking........$<"
	@$(LD) $(LDFLAGS) -o $@ $(OBJ)

unload:
	@echo "		remove real time task......"
	(./rmrtl)

load:
	@make unload
	@echo "		Before load  real time task......"
	(./insrtl)
