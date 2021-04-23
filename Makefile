CFLAGS+= -Wall -Werror -std=gnu99 -g
LDFLAGS=-pthread

BINARIES=prga-hw08-main

all: $(BINARIES)

OBJS=$(patsubst %.c,%.o,$(wildcard *.c))
OBJ_OI=prg_io_nonblock.o

prga-hw08-main : prga-hw08-main.o $(OBJ_OI) 
	$(CC) $< $(OBJ_OI) $(LDFLAGS) -o $@

$(OBJS): %.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -f $(BINARIES) $(OBJS)
