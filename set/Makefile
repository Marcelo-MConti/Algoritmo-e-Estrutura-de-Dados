.POSIX:

CC = cc

DEFCFLAGS = -Wall -Wextra -std=c99 $(CFLAGS)
DEFLDFLAGS = $(LDFLAGS)

OBJ = avl.o rb.o set.o main.o
OUT = set

all: $(OUT)

run: $(OUT)
	./$(OUT)

clean:
	rm -f $(OBJ) $(OUT)

dist:
	zip -r ../$(OUT).zip . -i Makefile '*.c' '*.h'

.c.o:
	$(CC) $(DEFCFLAGS) -c $< -o $@

$(OUT): $(OBJ)
	$(CC) $(DEFCFLAGS) $(OBJ) $(DEFLDFLAGS) -o $@
