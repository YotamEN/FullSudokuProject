#ifndef SUDOKU_C_DS_H
#define SUDOKU_C_DS_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct sudoku_board Sudoku;

struct sudoku_board{
    int **actual_board;
    int **rows;
    int **columns;
    int ***blocks;
    int **fixed_cells;
    int **err_cells;
};

Sudoku* createBoard(int row, int block);

Sudoku* copyBoard(Sudoku* board);

void copyCurrentBoard(Sudoku* copyFrom, Sudoku* copyTo);

void destroyBoard(Sudoku* board);

void printBoard(Sudoku* board);

int set(Sudoku *board, int column, int row, int value);

int get(Sudoku* board, int column, int row);

int isValid(Sudoku* board, int column, int row, int value);

void free2DArray(int **arr, int size_of_arr);

void copy2DArray(int **A, int **B, int size_of_A, int size_of_B);

void initialize2DArray(int** arr);

void initializeBoard(Sudoku* board);

void setGameMode(int mode);

int getGameMode();

void setMarkErrors(int mode);

int getMarkErrors();

void setRowSize(int size);

int getRowSize();

void setColumnSize(int size);

int getColumnSize();

int getErrBoard();

int advanceMove();

int prevMove();

void toDefault();

#endif
