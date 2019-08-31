#include "game.h"

char* checkModeName(int mode_code);
void wrongModeForFuncPrint(int mode_code, int allowed_a, int allowed_b);
int checkAvailableInMode(int allowed_mode_a, int allowed_mode_b);


/* --------------------------------------------------
 * ------------ User and Command Functions ----------
 * --------------------------------------------------
 */

/*
 * if reached "exit" or encountered EOF, return
 *
 */
void game(){

    int game_code;
    Sudoku *board = NULL, *solvedBoard = NULL;
    RESTART: while(1){
        setGameMode(INIT_MODE);

        while(1){
            printf("Enter your command:\n");
            game_code = get_command(&board, &solvedBoard);
            switch (game_code){
                case 0: /* game over - user won */
                    WIN_MSG;
                    goto RESTART;
                case 2:/* exit */
                    destroyPreExit(board, solvedBoard);
                    return;
                default:
                    ;
            }
            if (getGameMode()!= INIT_MODE) printBoard(board);
        }
    }
}
/*
 * str_len(command) == 10
 * if command == 2 : "exit" return 2
 * else if command == 1 : "restart" return 3
 * else if command == 3 : "set" AND user wins return 0
 */
int execute_command(cmd command, Sudoku **board, Sudoku **solvedBoard) {
    Sudoku* validatedBoard;

    switch(command.name) {
        case e_solve:
            setGameMode(SOLVE_MODE);
            *board = loadBoard(command.address);
            *solvedBoard = loadBoard(command.address);
            break;

        case e_edit:
            setGameMode(EDIT_MODE);
            if (command.address[0] == '\0') {
                *board = createBoard(9,9);
                *solvedBoard = createBoard(9,9);
            } else{
                *board = loadBoard(command.address);
                *solvedBoard = loadBoard(command.address);
            }
            break;

        case e_mark_errors:
            if (checkAvailableInMode(SOLVE_MODE,0)){
                break;
            }
            if (command.x != 0 && command.x != 1){
                printf("ERROR: function accepts either 1 or 0\n");
                break;
            }
            setMarkErrors(command.x);
            break;

        case e_print_board:
            /* board gets printed anyway...*/
            if (checkAvailableInMode(SOLVE_MODE, EDIT_MODE)){
                break;
            }
            break;

        case e_set:
            /*return 0 if user won*/
            if (checkAvailableInMode(SOLVE_MODE, EDIT_MODE)){
                break;
            }
            if (getGameMode() == SOLVE_MODE && isFixed(*board, command.x, command.y)){
                printf("Cell (%d, %d) is fixed. Fixed cells can only be changed in edit mode.\n", command.x, command.y);
                return 1;
            }
            return setZtoXY(*board, command);

        case e_validate:
            if (checkAvailableInMode(SOLVE_MODE, EDIT_MODE)){
                break;
            }
            if (getErrBoard()){
                printf("The board is erroneous - correct all errors before validating\n");
                break;
            }
            /* FIXME - GUROBI*/
            validatedBoard = validateCurrentBoard(*board, *solvedBoard);
            copyCurrentBoard(validatedBoard, *solvedBoard);
            destroyBoard(validatedBoard);
            break;

        case e_guess:
            if (checkAvailableInMode(SOLVE_MODE,0)){
                break;
            }
            if (getErrBoard()){
                printf("The board is erroneous - correct all errors before guessing\n");
                break;
            }
            /*  TODO  */
            break;

        case e_generate:
            if (checkAvailableInMode(EDIT_MODE,0)){
                break;
            }
            /*  TODO  */
            break;

        case e_undo:
            if (checkAvailableInMode(SOLVE_MODE, EDIT_MODE)){
                break;
            }
            if (undoMove(*board)){
                printf("ERROR: 'undo' unavailable, already at starting point\n");
                break;
            }
            prevMove();
            break;

        case e_redo:
            if (checkAvailableInMode(SOLVE_MODE, EDIT_MODE)){
                break;
            }
            /*Omer fix */
            if (advanceMove()){
                printf("ERROR: 'redo' unavailable, already at ending point\n");
                break;
            }
            redoMove(*board);
            break;

        case e_save:
            if (checkAvailableInMode(SOLVE_MODE, EDIT_MODE)){
                break;
            }
            if (getGameMode() == SOLVE_MODE){
                if (getErrBoard()){
                    printf("The board is erroneous - correct all errors before saving\n");
                    break;
                }
                /* TODO - validate board and if no solution - saving not allowed! remember to break;*/
            }
            saveBoard(*board, command.address);
            break;

        case e_hint:
            if (checkAvailableInMode(SOLVE_MODE,0)){
                break;
            }
            if (getErrBoard()){
                printf("The board is erroneous - correct all errors before asking for a hint\n");
                break;
            }
            if (isFixed(*board, command.x, command.y)){
                printf("This cell is fixed! Hint: try a different cell\n");
                break;
            }
            if (get(*board, command.x, command.y) != 0){
                printf("This cell already has a value! Hint: try a different cell\n");
                break;
            }
            /*FIXME - run ILP. if unsolvable - ERROR!*/
            giveHint(command.x,command.y,*solvedBoard);
            break;

        case e_guess_hint:
            if (checkAvailableInMode(SOLVE_MODE,0)){
                break;
            }
            if (getErrBoard()){
                printf("The board is erroneous - correct all errors before asking for a hint\n");
                break;
            }
            if (isFixed(*board, command.x, command.y)){
                printf("This cell is fixed! Hint: try a different cell\n");
                break;
            }
            if (get(*board, command.x, command.y)){
                printf("This cell already has a value! Hint: try a different cell\n");
                break;
            }
            /*  TODO - the whole damn thing */
            break;

        case e_num_solutions:
            if (checkAvailableInMode(SOLVE_MODE, EDIT_MODE)){
                break;
            }
            /*  TODO  */
            break;

        case e_autofill:
            if (checkAvailableInMode(SOLVE_MODE,0)){
                break;
            }
            autofillBoard(*board);
            break;

        case e_reset:
            if (checkAvailableInMode(SOLVE_MODE, EDIT_MODE)){
                break;
            }
            reset(*board);
            return 3;

        case e_exit:
            EXIT_MSG1;
            return 2;

        default:
            ERROR_MSG3;
    }
    return 1;

}
/* -----------------------------------------
 * ------------ Feature Functions ----------
 * -----------------------------------------
 */

/* - unnecessary code for now FIXME
Sudoku* fillConstCells(Sudoku *board, int permCells) {
    int i,x,y;
    Sudoku* gameBoard = createBoard(getRowSize()*getColumnSize(), getRowSize()*getColumnSize());

    for (i=0; i<permCells; i++){
        do {
            x = (rand() % (getRowSize()*getColumnSize()))+1;
            y = (rand() % (getRowSize()*getColumnSize()))+1;
        }while(gameBoard->fixed_cells[y-1][x-1] == 1);
        set(gameBoard,x,y,get(board,x,y));
        gameBoard->fixed_cells[y-1][x-1] = 1;
    }
    return gameBoard;
}
*/

void destroyPreExit(Sudoku* A, Sudoku* B){
    destroyBoard(A);
    destroyBoard(B);
}

void giveHint(int x, int y, Sudoku* solvedBoard){
    int clue = get(solvedBoard,x,y);
    HINT_MSG(clue);
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

int setZtoXY(Sudoku *board, cmd command) {
    int N = getColumnSize()*getRowSize();
    int unsuccessful = 0;

    unsuccessful = set(board, command.x, command.y, command.z);
    if (unsuccessful == 1) {printf("Sudoku board is NULL - exiting...\n"); return 2;}
    else if(unsuccessful == 2) {ERROR_MSG1; return 1;}
    /*Check for a win:*/
    if (getFilledCells() == (N*N)){
        if (getErrBoard() == 0) return 0;
        else printf("The solution is erroneous - Fix all errors to finish\n");
    }
    /* printf("set success = %d\n",success); */
    return 3;
}


/* ---------------------------------------
 * ------------ Private Functions ----------
 * ---------------------------------------
 */
char* checkModeName(int mode_code){
    char *mode;

    switch(mode_code){
        case(INIT_MODE):
            mode = "init";
            break;
        case(SOLVE_MODE):
            mode = "solve";
            break;
        case(EDIT_MODE):
            mode = "edit";
            break;
        default:
            mode ="";
    }
    return mode;
}


void wrongModeForFuncPrint(int mode_code, int allowed_a, int allowed_b){
    char *mode, *alw_mode_a, *alw_mode_b;

    mode = checkModeName(mode_code);
    alw_mode_a = checkModeName(allowed_a);
    alw_mode_b = checkModeName(allowed_b);
    printf("This function is not available in %s mode, only in modes: %s %s\n ", mode, alw_mode_a, alw_mode_b);
}

int checkAvailableInMode(int mode_a, int mode_b){
    int mode = getGameMode();
    if (mode != mode_a && mode != mode_b){
        wrongModeForFuncPrint(mode, mode_a, mode_b);
        return 1;
    }
    return 0;
}






