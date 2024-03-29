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

typedef struct undo_redo_move UndoRedoMove;
typedef struct move Move;
typedef struct autofill_move AutoFill;

struct undo_redo_move{
    struct move* current_move;
    struct undo_redo_move* next_move;
    struct undo_redo_move* prev_move;
};


struct move{
    int row;
    int column;
    int from;
    int to;
    struct move* next;
    struct move* prev;
};


struct autofill_move{
    int row;
    int column;
    int value;
    struct autofill_move* next;
};

Sudoku* createBoard(int row, int column);

Sudoku* copyBoard(Sudoku* board);

void copyCurrentBoard(Sudoku* copyFrom, Sudoku* copyTo);

void destroyBoard(Sudoku* board);

void printBoard(Sudoku* board);

void freeMoves(UndoRedoMove* move);

int set(Sudoku *board, int column, int row, int value, int isMove);

int get(Sudoku* board, int column, int row);

int isValid(Sudoku* board, int column, int row, int value);

void free2DArray(int **arr, int size_of_arr);

void copy2DArray(int **A, int **B, int size_of_A, int size_of_B);

void initialize2DArray(int** arr);

void initializeBoard(Sudoku* board);

int redoMove(Sudoku* board);

int undoMove(Sudoku* board);

void reset(Sudoku* board);

void saveBoard(Sudoku* board, char* file);

Sudoku* loadBoard(char* file);

void autofillBoard(Sudoku* board);

void setGameMode(int mode);

int getGameMode();

void setMarkErrors(int mode);

int getMarkErrors();

void setRowSize(int size);

int getRowSize();

void setColumnSize(int size);

int getColumnSize();

int getErrBoard();

int getFilledCells();

int advanceMove();

int prevMove();

int nextMove();

void toDefault();

void saveBoard(Sudoku* board, char* file);

Sudoku* loadBoard(char* file);

int isFixed(Sudoku* board, int column, int row);

void initializeInnerMove(Move* move);

void initializeMove(UndoRedoMove* move);

UndoRedoMove* getFirstMove();
#endif
