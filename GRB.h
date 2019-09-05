
#ifndef SUDOKU_C_GRB_H
#define SUDOKU_C_GRB_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "DS.h"
#include "parser.h"
#include "c:/gurobi811/win64/include/gurobi_c.h"

int GurobiSolution(Sudoku *board, Sudoku *solvedBoard, int is_integer_prog, float threshold, cmd command);


#endif /*SUDOKU_C_GRB_H*/

