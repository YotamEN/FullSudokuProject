CC = gcc
OBJS = main.o game.o parser.o solver.o DS.o GRB.o

EXEC = sudoku-console
COMP_FLAG = -ansi -Wall -Wextra -Werror -pedantic-errors
GUROBI_COMP = -I/usr/local/lib/gurobi563/include
GUROBI_LIB = -L/usr/local/lib/gurobi563/lib -lgurobi56

$(EXEC): $(OBJS)
	$(CC) $(OBJS) $(GUROBI_LIB) -o $@
main.o: main.c game.h
	$(CC) $(COMP_FLAG) $(GUROBI_COMP) -c $*.c
DS.o: DS.c DS.h
	$(CC) $(COMP_FLAG) -c $*.c
solver.o: solver.c solver.h DS.h
	$(CC) $(COMP_FLAG) -c $*.c
parser.o: parser.c parser.h DS.h
	$(CC) $(COMP_FLAG) $(GUROBI_COMP) -c $*.c
GRB.o: GRB.c GRB.h DS.h parser.h 
	$(CC) $(COMP_FLAG) $(GUROBI_COMP) -c $*.c
game.o: game.c game.h parser.h DS.h solver.h GRB.h
	$(CC) $(COMP_FLAG) $(GUROBI_COMP) -c $*.c
clean:
	rm -f *.o $(EXEC)

