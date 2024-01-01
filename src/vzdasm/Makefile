
.PHONY: all clean

CFLAGS = -Wall -Wextra

all: vzdasm

vzdasm: vzdasm.c
	$(CC) $(CFLAGS) -o vzdasm vzdasm.c

clean:
	rm -f vzdasm
