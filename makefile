CC = gcc
OBJS = main.o game.o parser.o solver.o DS.o
EXEC = sudoku
COMP_FLAG = -ansi -Wall -Wextra -Werror -pedantic-errors

$(EXEC): $(OBJS)
	$(CC) $(OBJS) -o $@
main.o: main.c DS.h solver.h parser.h game.h
	$(CC) $(COMP_FLAG) -c $*.c
game.o: game.c game.h DS.h solver.h parser.h
	$(CC) $(COMP_FLAG) -c $*.c
parser.o: parser.c parser.h game.h DS.h
	$(CC) $(COMP_FLAG) -c $*.c
solver.o: solver.c solver.h DS.h
	$(CC) $(COMP_FLAG) -c $*.c
DS.o: DS.c DS.h
	$(CC) $(COMP_FLAG) -c $*.c
clean:
	rm -f $(OBJS) $(EXEC)
