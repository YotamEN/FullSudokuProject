#ifndef SUDOKU_C_PARSER_H
#define SUDOKU_C_PARSER_H
#define MoreWords (token!=NULL)
#define delimiter " \t\n"
#define DEBUG_MSG(i) printf("I'm here~! %d\n",(i));
#define EXIT_MSG1 printf("Exiting...\n")
#define EXIT_MSG2(func) printf("Error: %s has failed\n", (func))
#define ERROR_MSG1 printf("Error: cell is fixed\n")
#define ERROR_MSG3 printf("Error: invalid command\n")
#define WIN_MSG printf("Puzzle solved successfully\n")
#define HINT_MSG(clue) printf("Hint: set cell to %d\n",clue)
#define VAL_FAIL_MSG printf("Validation failed: board is unsolvable\n")
#define VAL_PASS_MSG printf("Validation passed: board is solvable\n")
#include "game.h"
#include "DS.h"

#include <stdio.h>
#include <string.h>

enum cmd_name {e_solve, e_edit, e_mark_errors, e_print_board, e_set, e_validate, e_guess, e_generate, e_undo, e_redo,
        e_save, e_hint, e_guess_hint, e_num_solutions, e_autofill, e_reset, e_exit, e_unknown} cmd_name;

typedef struct commands{
    enum cmd_name   name        ;
    char*           address     ;
    int             x           ;
    int             y           ;
    int             z           ;
    float           f           ;
} cmd;

enum cmd_name checkCommand(char* command);
void clear();
int get_command(Sudoku *board, Sudoku *solvedBoard);

#endif
