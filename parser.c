#include "parser.h"
/* -------- Declaration of private funcs -------- */
int     stringToInt               (char a[])                                ;
int     someProblem               (int count, cmd command)                  ;
void    unknownCommandMessage     ()                                        ;
void    wrongNumberOfParamsMessage(int wrong, int right)                    ;
void    wrongValuesMessage        (int num, int min_right,
                                    int max_right, int num_param)           ;
void    wrongValuesMessageFloat   (float num, int min_right, int max_right) ;
float   stringToFloat             (char* s)                                 ;

/*
 * receive command from user and call relevant function
 * if returns 2 -> exit!
 * if returns 3 -> retry!
 */
int get_command(Sudoku **board, Sudoku **solvedBoard) {
    /* ----- field members -----*/
    char    comm_line[Line_Length], *token, *s  ;
    cmd     command    ;
    int     count = 1  ;
    /* ----- process command -----*/
    if (fgets(comm_line,Line_Length,stdin) == NULL){
        EXIT_MSG1   ;
        return 2    ;
    }
    if((token = strtok(comm_line,delimiter)) == NULL){
        EXIT_MSG1   ;
        return 1    ;
    }
    command.name    = checkCommand(token);
    command.address = ""                 ;
    command.x       = -1                 ;
    command.y       = -1                 ;
    command.z       = -1                 ;
    command.f       = -1.0               ;

    if (command.name == e_unknown){
        unknownCommandMessage();
        return 1;
    }

    while MoreWords{
        token = strtok(NULL,delimiter);
        if(token == NULL) break;
        count++;
        if(count==2){
            s = token;
            if (command.name == e_edit || command.name == e_save || command.name == e_solve){
                command.address = malloc(sizeof(char)*strlen(token));
                strcpy(command.address , token);
            }
            else if (command.name == e_guess){
                command.f = stringToFloat(s);
            }
            else
                command.x = stringToInt(s);
        }
        else if(count==3) command.y = stringToInt(token);
        else if(count==4) command.z = stringToInt(token);
    }
    if (someProblem(count-1, command)) return 1;
    return execute_command(command, board, solvedBoard);
}
/* --------- */
enum cmd_name checkCommand(char* command){
    if(strcmp(command, "solve")==0){
         return e_solve;
     }
    else if(strcmp(command, "edit")==0){
         return e_edit;
     }
    else if (strcmp(command, "mark_errors")==0){
         return e_mark_errors;
     }
    else if(strcmp(command, "print_board")==0){
         return e_print_board;
     }
    else if(strcmp(command, "set")==0){
        return e_set;
    }
    else if(strcmp(command, "validate")==0){
        return e_validate;
     }
    else if(strcmp(command, "guess")==0){
        return e_guess;
    }else if(strcmp(command, "generate")==0){
        return e_generate;
    }else if(strcmp(command, "undo")==0){
        return e_undo;
    }else if(strcmp(command, "redo")==0){
        return e_redo;
    }else if(strcmp(command, "save")==0){
        return e_save;
    }else if(strcmp(command, "hint")==0){
        return e_hint;
    }else if(strcmp(command, "guess_hint")==0){
        return e_guess_hint;
    }else if(strcmp(command, "num_solutions")==0){
        return e_num_solutions;
    }else if(strcmp(command, "autofill")==0){
        return e_autofill;
    }else if(strcmp(command, "reset")==0){
        return e_reset;
    }else if(strcmp(command, "exit")==0){
        return e_exit;
    }
    else return e_unknown;
}

/*
 *  -------------------------------------
 *  -----   PRIVATE     FUNCTIONS   -----
 *  -------------------------------------
 */

/*
 * This function is meant to check if the command given by the user is valid.
 * If some problem occurs, the user will have to re-enter his command.
 */
int someProblem(int count, cmd command){
    int N = getRowSize() * getColumnSize();
    int free_cells = N*N - getFilledCells();

    /* commands with file path as argument*/
    if (        command.name == e_edit      || command.name == e_save   || command.name == e_solve){
        if (count != 1 && command.name != e_edit){
            wrongNumberOfParamsMessage(count, 1);
            return 1;
        }
        if (command.name != e_edit && command.address[0] == '\0'){
            printf("Path or file name is missing, please provide one for this function to work\n");
            return 1;
        }

    }
    /* commands with no arguments */
    else if (   command.name == e_validate  || command.name == e_undo   || command.name == e_num_solutions
           ||   command.name == e_redo      || command.name == e_exit   || command.name == e_autofill
           ||   command.name == e_reset     || command.name == e_print_board ){
        if (count > 1)  return 1;
    }
    /* commands with 1 argument */
    else if (   command.name == e_mark_errors ){
        if (count > 1){
            wrongNumberOfParamsMessage(count, 1);
	    printf("(You could also enter 0 parameters and flip the current state)\n");
            return 1;
        }
        if (notInRange(command.x, 0, 1)){
            wrongValuesMessage(command.x, 0, 1, 1);
            return 1;
        }
    }
    else if (   command.name == e_guess ) {
        if (count != 1){
            wrongNumberOfParamsMessage(count, 1);
            return 1;
        }
        if (command.f < 0 || command.f > 1){
            wrongValuesMessageFloat(command.f, 0, 1);
            return 1;
        }
    }
    /* commands with 2 arguments */
    else if (   command.name == e_guess_hint || command.name == e_hint  || command.name == e_generate){
        if (count != 2){
            wrongNumberOfParamsMessage(count, 2);
            return 1;
        }
        if (command.name == e_generate){
            if (notInRange(command.x, 0, free_cells)){
                wrongValuesMessage(command.x, 0, free_cells, 1);
                printf("(There are only %d free cells on the board\n)", free_cells);
                return 1;
            }
            if (notInRange(command.y, 0, N*N)){
                wrongValuesMessage(command.y, 0, N*N, 2);
                return 1;
            }
        }
        else {
            if (notInRange(command.x, 1, N)) {
                wrongValuesMessage(command.x, 1, N, 1);
                return 1;
            }
            if (notInRange(command.y, 1, N)) {
                wrongValuesMessage(command.y, 1, N, 2);
                return 1;
            }
        }
    }
    /* commands with 3 arguments */
    else if (   command.name == e_set ){
        if (count != 3){
            wrongNumberOfParamsMessage(count, 3);
            return 1;
        }
        if (notInRange(command.x, 1, N)) {
            wrongValuesMessage(command.x, 1, N, 1);
            return 1;
        }
        if (notInRange(command.y, 1, N)) {
            wrongValuesMessage(command.y, 1, N, 2);
            return 1;
        }
        if (notInRange(command.z, 0, N)) {
            wrongValuesMessage(command.z, 0, N, 3);
            return 1;
        }
    }
    return 0;
}

float stringToFloat(char* s){
    float rez = 0, fact = 1;
    int point_seen, d;

    if (*s == '-'){
        s++;
        fact = -1;
    }
    for (point_seen = 0; *s; s++){
        if (*s == '.'){
            point_seen = 1;
            continue;
        }
        d = *s - '0';
        if (d >= 0 && d <= 9){
            if (point_seen) fact /= 10.0f;
            rez = rez * 10.0f + (float)d;
        }
    }
    return rez * fact;
}

int stringToInt(char a[]) {
    int c, sign, offset, n;
    sign = 0;

    if (a[0] == '-') {  /* Handle negative integers */
        sign = -1;
    }

    if (sign == -1) {  /* Set starting position to convert */
        offset = 1;
    }
    else {
        offset = 0;
    }

    n = 0;

    for (c = offset; a[c] != '\0'; c++) {
        n = n * 10 + a[c] - '0';
    }

    if (sign == -1) {
        n = -n;
    }

    return n;
}

/*
 *  ----------------------------------
 *  -----   PRINT    FUNCTIONS   -----
 *  ----------------------------------
 */
int notInRange(int num, int min, int max){
    return (!(num >= min && num <= max));
}
void unknownCommandMessage(){
    printf("ERROR: You have entered an unknown command. Please try again\n");
}

void wrongNumberOfParamsMessage(int wrong, int right){
    printf("ERROR: You have entered %d parameters, the correct number of parameters for this function: %d\n",wrong,right);
}

void wrongValuesMessage(int num, int min_right, int max_right, int num_param) {
    printf("ERROR: You have entered an illegal value: %d, to parameter number %d.\n"
           "Please enter a value between %d and %d\n",num,num_param,min_right,max_right);
}

void wrongValuesMessageFloat(float num, int min_right, int max_right){
    printf("ERROR: You have entered an illegal value: %f. Please enter a value between %d.0 and %d.0\n",num,min_right,max_right);
}
