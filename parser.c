#include "parser.h"

/*
 * recieves from user how many filled cells he wants
 *  number must be in [0,80]
 */
int num_of_cells_to_fill(){
    int cells;
    while (1){
        printf("Please enter the number of cells to fill [0-80]:\n");
        if (scanf("%d",&cells) <= 0) {
            if(!feof(stdin))
                printf("Error: not a number\n");
            EXIT_MSG1;
            exit(1);
        }

        if(cells>=0 && cells<=80){ /*clear();*/ return cells;}
        printf("Error: invalid number of cells to fill (should be between 0 and 80)\n");

    }
}

/*
 * receive command from user and call relevant function
 * if returns 2 -> exit!
 * if returns 3 -> retry!
 */
int get_command(int won, Sudoku *board, int *sets, Sudoku *solvedBoard) {
    /* ----- field members -----*/
    char comm_line[Line_Length], *token;
    cmd command;
    int count=1;
    /* ----- process command -----*/
    if (fgets(comm_line,Line_Length,stdin) == NULL){EXIT_MSG1; return 2;}
    if((token = strtok(comm_line,delimiter)) == NULL){
        EXIT_MSG1;
        return 1;
    }
    command.name = checkCommand(token);
    while MoreWords{
        token = strtok(NULL,delimiter);
        if(token == NULL) break;
        count++;
        if(count==2){
            if (command.name == e_edit || command.name == e_save){
                strcpy(command.address , token);
                break;
            }
            else
                command.x = (token[0]-48);
        }
        else if(count==3) command.y = (token[0]-48);
        else if(count==4) { command.z = (token[0]-48); break;}
    }
    return execute_command(command, board, won, sets, solvedBoard);
}
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
/* clear upto newline */
void clear(){
    scanf("%*[^\n]");
    scanf("%*c");
}
