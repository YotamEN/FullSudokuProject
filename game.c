#include "game.h"
/* ----------------------------------------
 * ------------ User Functions ----------
 * ----------------------------------------
 */
/*
 * if reached "exit" or encountered EOF, return 2
 *
 */
int game(){

    int win, const_cells, game_code, sets;
    Sudoku *board, *solvedBoard;
    RESTART: while(1){
        if ((const_cells = num_of_cells_to_fill()) == -1) return 2;
        sets=const_cells;
        win = 0;
        if ((solvedBoard = generatePuzzle()) == NULL){ EXIT_MSG2("generatePuzzle()"); return 2;}
        board = fillConstCells(solvedBoard, const_cells);
        printBoard(board);
        while(1){

            game_code = get_command(win, board, &sets, solvedBoard);
            switch (game_code){
                case 0:
                    WIN_MSG;
                    win =1;
                    break;
                case 2:
                    destroyPreExit(board, solvedBoard);
                    return 2;
                case 3:
                    destroyPreExit(board, solvedBoard);
                    goto RESTART;
                case 4:
                    printBoard(board);
                    break;
                default:
                    break;
            }
        }
    }
}
/*
 * str_len(command) == 10
 * if command == 2 : "exit" return 2
 * else if command == 1 : "restart" return 3
 * else if command == 3 : "set" AND user wins return 0
 */
int execute_command(cmd command, Sudoku *board, int won, int *sets, Sudoku *solvedBoard) {
    Sudoku* validatedBoard;

    switch(command.name){
        case e_solve:
            /*  TODO  */
            break;

        case e_edit:
            /*  TODO  */
            break;

        case e_mark_errors:
            /*  TODO  */
            break;

        case e_print_board:
            /*  TODO  */
            break;

        case e_set:
            /*  TODO  */
            /* check if game won. if yes: win = 1;*/
            return setZtoXY(board, command, sets);

        case e_validate:
            validatedBoard = validateCurrentBoard(board, solvedBoard);
            copyCurrentBoard(validatedBoard, solvedBoard);
            /*destroyBoard(validatedBoard);*/
            return 1;
            break;

        case e_guess:
            /*  TODO  */
            break;

        case e_generate:
            /*  TODO  */
            break;

        case e_undo:
            /*  TODO  */
            break;

        case e_redo:
            /*  TODO  */
            break;

        case e_save:
            /*  TODO  */
            break;

        case e_hint:
            giveHint(command.x,command.y,solvedBoard);
            return 1;
            break;

        case e_guess_hint:
            /*  TODO  */
            break;

        case e_num_solutions:
            /*  TODO  */
            break;

        case e_autofill:
            /*  TODO  */
            break;

        case e_reset:
            /*  TODO  */
            return 3; /* ? */

        case e_exit:
            EXIT_MSG1;
            return 2;

        default:
            ERROR_MSG3;
            return 1;
    }

}

Sudoku* fillConstCells(Sudoku *board, int permCells) {
    int i,x,y;
    Sudoku* gameBoard = createBoard(row_size,block_size);
    for (i=0; i<permCells; i++){
        do {
            x = (rand() % row_size)+1;
            y = (rand() % row_size)+1;
        }while(gameBoard->fixed_cells[y-1][x-1] == 1);
        set(gameBoard,x,y,get(board,x,y));
        gameBoard->fixed_cells[y-1][x-1] = 1;
    }
    return gameBoard;
}

void destroyPreExit(Sudoku* A, Sudoku* B){
    destroyBoard(A);
    destroyBoard(B);
}

void giveHint(int x, int y, Sudoku* solvedBoard){
    int clue = get(solvedBoard,x,y);
    HINT_MSG;
}

Sudoku * validateCurrentBoard(Sudoku *board, Sudoku *solvedBoard){
    Sudoku* temp = solve(board);
    if (temp == NULL){
        VAL_FAIL_MSG;
        return solvedBoard;
    }
    else{
        VAL_PASS_MSG;
        /*destroyBoard(solvedBoard);*/
        return temp;
    }
}

int setZtoXY(Sudoku *board, cmd command, int *sets) {
    int success = 0;
    if(command.x == -1 || command.y == -1 || command.z == -1){
        ERROR_MSG3;
        return 1;
    }

    success = set(board, command.x, command.y, command.z);
    if(success == 2) {ERROR_MSG1; return 1;}
    else if(success == 3) {ERROR_MSG2; return 1;}
    /*printf("set success = %d\n",success);*/
    *sets += success;
    if (*sets == row_size*row_size) return 0; /* we have a winner! */
    return 4;
}
/* -----------------------------------------
 * ------------ Private Functions ----------
 * -----------------------------------------
 */



