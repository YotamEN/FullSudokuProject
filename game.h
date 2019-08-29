#ifndef SUDOKU_C_GAME_H
#define SUDOKU_C_GAME_H

#define Line_Length 1025
#include "parser.h"
#include "DS.h"
#include "solver.h"

Sudoku * validateCurrentBoard(Sudoku *board, Sudoku *solvedBoard);
int execute_command(cmd command, Sudoku *board, int won, int *sets, Sudoku *solvedBoard);
int game();
void destroyPreExit(Sudoku* A, Sudoku* B);
void giveHint(int x, int y, Sudoku* solvedBoard);
int setZtoXY(Sudoku *board, cmd command, int *sets);
Sudoku * fillConstCells(Sudoku *board, int permCells);


#endif
