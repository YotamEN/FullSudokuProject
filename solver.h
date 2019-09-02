#ifndef SUDOKU_C_SOLVER_H
#define SUDOKU_C_SOLVER_H

#include "DS.h"
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

Sudoku* solve(Sudoku* board);

Sudoku* solveBoard(Sudoku* board, int column, int row, int isBacktrack);

Sudoku* generatePuzzle();

int* findBacktrackCell(Sudoku* board, int column, int row);

Sudoku* solveRandBoard(Sudoku* board, int column, int row);

int exhaustiveBacktracking(Sudoku* board);

void makeFixed(Sudoku* board);

/*int* validValues(Sudoku* board, int column, int row);*/
#endif
