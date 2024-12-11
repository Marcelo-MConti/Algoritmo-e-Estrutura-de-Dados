.POSIX:
.SUFFIXES: .in .out .ok

CC = cc

DEFCFLAGS = -Wall -Wextra -std=c99 $(CFLAGS)
DEFLDFLAGS = $(LDFLAGS)

OBJ = avl.o rb.o set.o main.o
OUT = set

all: $(OUT)

run: $(OUT)
	./$(OUT)

clean:
	make -C tests clean
	rm -f $(OBJ) $(OUT)

dist:
	zip -r ../$(OUT).zip . -i relatorio.pdf Makefile tests/Makefile '*.c' '*.h' '*.in' '*.out'

.c.o:
	$(CC) $(DEFCFLAGS) -c $< -o $@

$(OUT): $(OBJ)
	$(CC) $(DEFCFLAGS) $(OBJ) $(DEFLDFLAGS) -o $@

test:
	make -C tests OUT=$(OUT) test
