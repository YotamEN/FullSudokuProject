#include "game.h"

char* checkModeName(int mode_code);
void wrongModeForFuncPrint(int mode_code, int allowed_a, int allowed_b);
int checkAvailableInMode(int allowed_mode_a, int allowed_mode_b);
int checkIfContains(int* array, int num, int size);
int leaveYCells(Sudoku* board, int y);
int fillRandX(Sudoku* board, int x);


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
    toDefault();
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
                    if(board != NULL && solvedBoard != NULL) /*Omer fix*/
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
    Sudoku *tempBoard;
    int solvable, freecells, i, error;
    int N = getColumnSize()*getRowSize();
    if(getGameMode() == INIT_MODE)
        toDefault();
    switch(command.name) {
        case e_solve:
            setGameMode(SOLVE_MODE);
            *board       = loadBoard(command.address);
            *solvedBoard = copyBoard(*board)         ;
            break;

        case e_edit:
            setGameMode(EDIT_MODE);
            if (command.address[0] == '\0') {
                *board       = createBoard(3,3);
                *solvedBoard = createBoard(3,3);
            } else{
                *board       = loadBoard(command.address);
                *solvedBoard = copyBoard(*board)         ;
            }
            break;

        case e_mark_errors:
            if (checkAvailableInMode(SOLVE_MODE,0)){
                break;
            }
	    if (command.x == -1){
		setMarkErrors(1-getMarkErrors());
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
            if(advanceMove()){
                perror("Memory allocation error.\n");
                exit(EXIT_FAILURE);
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
            solvable = GurobiSolution(*board, *solvedBoard, ILP, 0, command);
            if(solvable == 0) printf("Board is solvable - keep going!\n");
            else if (solvable == 2) printf("Board unsolvable!\n");
	    else printf("validation failed..\n");
            break;

        case e_guess:
            if (checkAvailableInMode(SOLVE_MODE,0)){
                break;
            }
            if (getErrBoard()){
                printf("The board is erroneous - correct all errors before guessing\n");
                break;
            }
            if(advanceMove()){
                perror("Memory allocation error.\n");
                exit(EXIT_FAILURE);
            }
            GurobiSolution(*board, *solvedBoard, LP, command.f, command);
            break;

        case e_generate:
            if (checkAvailableInMode(EDIT_MODE,0)){
                break;
            }
            if(advanceMove()){
                perror("Memory allocation error.\n");
                exit(EXIT_FAILURE);
            }
            freecells = N*N - getFilledCells();
            if (freecells < command.x){
                printf("ERROR: There are only %d free cells on the board\n", freecells);
                break;
            }
            copyCurrentBoard(*board, *solvedBoard);
            tempBoard = createBoard(getRowSize(), getColumnSize());
	    error =1;
            for(i=0; i<1000; i++) {
                copyCurrentBoard(*solvedBoard, tempBoard);
                if (fillRandX(tempBoard, command.x)) 
                    continue;
                if(GurobiSolution(*solvedBoard, tempBoard, ILP, 0, command))
                    continue;
                leaveYCells(tempBoard, command.y);
		copyCurrentBoard(tempBoard, *solvedBoard);
                copyCurrentBoard(*solvedBoard, *board);
                destroyBoard(tempBoard);
		error =0;
                break;
            }
            if (error){
	    printf("ERROR: Generation has failed.\n");
            destroyBoard(tempBoard);
	    }
            break;

        case e_undo:
            if (checkAvailableInMode(SOLVE_MODE, EDIT_MODE)){
                break;
            }
            if (prevMove()){
                printf("ERROR: 'undo' unavailable, already at starting point\n");
                break;
            }
            undoMove(*board);
            break;

        case e_redo:
            if (checkAvailableInMode(SOLVE_MODE, EDIT_MODE)){
                break;
            }
            /*Omer fix */
            if (nextMove()){
                printf("ERROR: 'redo' unavailable, already at ending point\n");
                break;
            }
            redoMove(*board);
            break;

        case e_save:
            if (checkAvailableInMode(SOLVE_MODE, EDIT_MODE)){
                break;
            }
            if (getGameMode() == EDIT_MODE){
                if (getErrBoard()){
                    printf("The board is erroneous - correct all errors before saving in 'edit' mode\n");
                    break;
                }
                if (getErrBoard() != 0){
                    printf("The board is unsolvable - fix it to save the board properly in 'edit' mode\n");
                    break;
                }
            }
            saveBoard(*board, command.address);
            break;

        case e_hint:
            if (checkAvailableInMode(SOLVE_MODE,0)){
                break;
            }
            if (getErrBoard()){
                printf("ERROR: The board is erroneous - correct all errors before asking for a hint\n");
                break;
            }
            if (isFixed(*board, command.x, command.y)){
                printf("ERROR: This cell is fixed! Hint: try a different cell\n");
                break;
            }
            if (get(*board, command.x, command.y) != 0){
                printf("ERROR: This cell already has a value! Hint: try a different cell\n");
                break;
            }
            if (GurobiSolution(*board, *solvedBoard, ILP, 0.0, command)){
                printf("ERROR: The board is unsolvable! Hint: fix the board\n");
                break;
            }
            giveHint(command.x,command.y,*solvedBoard);
            break;

        case e_guess_hint:
            if (checkAvailableInMode(SOLVE_MODE,0)){
                break;
            }
            if (getErrBoard()){
                printf("ERROR: The board is erroneous - correct all errors before asking for a hint\n");
                break;
            }
            if (isFixed(*board, command.x, command.y)){
                printf("ERROR: This cell is fixed! Hint: try a different cell\n");
                break;
            }
            if (get(*board, command.x, command.y)){
                printf("ERROR: This cell already has a value! Hint: try a different cell\n");
                break;
            }
            if(advanceMove()){
                perror("Memory allocation error.\n");
                exit(EXIT_FAILURE);
            }
            if (GurobiSolution(*board, *solvedBoard, LP, 0.0, command)){
                printf("ERROR: The board is unsolvable! Hint: fix the board\n");
                break;
            }
            break;

        case e_num_solutions:
            if (checkAvailableInMode(SOLVE_MODE, EDIT_MODE)){
                break;
            }
            printf("The number of possible solutions is: %d\n", exhaustiveBacktracking(*board));
            break;
        case e_autofill:
            if (checkAvailableInMode(SOLVE_MODE,0)){
                break;
            }
	    if(getErrBoard()){
		printf("ERROR: cannot use autofill erroneous board.\n");
		break;
	    }
            if(advanceMove()){
                perror("Memory allocation error.\n");
                exit(EXIT_FAILURE);
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
    if (command.address[0] != '\0')
        free(command.address);
    return 1;

}
/* -----------------------------------------
 * ------------ Feature Functions ----------
 * -----------------------------------------
 */


void destroyPreExit(Sudoku* A, Sudoku* B){
    destroyBoard(A);
    destroyBoard(B);
    freeMoves(getFirstMove());
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

    unsuccessful = set(board, command.x, command.y, command.z, 1);
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
    printf("This function is not available in %s mode, only in modes: %s %s\n", mode, alw_mode_a, alw_mode_b);
}

int checkAvailableInMode(int mode_a, int mode_b){
    int mode = getGameMode();
    if (mode != mode_a && mode != mode_b){
        wrongModeForFuncPrint(mode, mode_a, mode_b);
        return 1;
    }
    return 0;
}

int fillRandX(Sudoku* board, int x){
    int i,j,rndX, rndY, N;
    int found=0, count;
    N =getColumnSize()*getRowSize();
    for (i=0; i<x; i++){
        count =0;
        do {
            rndX = (rand() % N)+1;
            rndY = (rand() % N)+1;
            count++;
            if(count == BigNumber){
                printf("ERROR: Couldn't find an empty cell\n");
                return 1;
            }
        } while(get(board, rndX, rndY) != 0);
        for (j=1; j<=N; j++){
            if (isValid(board, rndX, rndY, j)){
                set(board, rndX, rndY, j, 1);
                found = 1;
                break;
            }
        }
        if(!found) return 1;
    }
    return 0;
}

int leaveYCells(Sudoku* board, int y){
    int rndX, rndY, N=getColumnSize()*getRowSize();
    int i, count;
    for (i=0; i<(N-y); i++){
        count =0;
        do {
            rndX = (rand() % N)+1;
            rndY = (rand() % N)+1;
            count++;
            if(count == BigNumber){
                printf("ERROR: Couldn't find a full cell\n");
                return 1;
            }
        } while(get(board, rndX, rndY) == 0);
        set(board, rndX, rndY,0, 1);
    }
    return 0;
}

int checkIfContains(int* array, int num, int size){
    int i;
    for (i=0; i<size; i++){
        if (array[i] == num) return 1;
    }
    return 0;
}






