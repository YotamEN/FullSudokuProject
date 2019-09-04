#include "DS.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int game_mode = 1;
int mark_errors = 1;
int row_size = 3;
int column_size = 3;
int err_board = 0;
int filled_cells = 0;

UndoRedoMove* first_move = NULL;
UndoRedoMove* current_move = NULL;

void toDefault(){
    mark_errors = 1;
    err_board = 0;
    filled_cells = 0;
    current_move = (UndoRedoMove*) malloc(sizeof(UndoRedoMove));
    if(current_move == NULL){
        perror("Memory allocation error.\n");
        exit(EXIT_FAILURE);
    }
    initializeMove(current_move);
    first_move = current_move;
}

Sudoku* createBoard(int row, int column){
    int i,j, N;
    int blocks_in_a_row, blocks_in_a_column;
	Sudoku* game_board = (Sudoku*) malloc(sizeof(Sudoku));
	N = row * column;

	if(game_board == NULL){ /*memory allocation failed*/
		return NULL;
	}

	/*initializing the actual board + handling memory allocation failures.*/
	game_board->actual_board = (int**)malloc(N * sizeof(int*));
	if(game_board->actual_board == NULL){
		free(game_board);
		return NULL;
	}
	for( i=0;i<N; ++i){
		game_board->actual_board[i] = (int*)malloc(N * sizeof(int));
		if(game_board->actual_board[i] == NULL){
            free2DArray(game_board->actual_board, 0);
			free(game_board);
			return NULL;
		}
	}

	/*initializing the rows array + handling memory allocation failures.*/
	game_board->rows = (int**)malloc(N * sizeof(int*));
	if(game_board->rows == NULL){
        free2DArray(game_board->actual_board, 0);
		free(game_board);
		return NULL;
	}

	for( i=0;i<N; ++i){
		game_board->rows[i] = (int*)malloc(N * sizeof(int));
		if(game_board->rows[i] == NULL){
            free2DArray(game_board->rows, 0);
            free2DArray(game_board->actual_board, 0);
			free(game_board);
			return NULL;
		}
	}
	/*initializing the columns array + handling memory allocation failures.*/
	game_board->columns = (int**)malloc(N * sizeof(int*));
	if(game_board->columns == NULL){
        free2DArray(game_board->rows, 0);
        free2DArray(game_board->actual_board, 0);
		free(game_board);
		return NULL;
	}
	for(i=0;i<N; ++i){
		game_board->columns[i] = (int*)malloc(N * sizeof(int));
		if(game_board->columns[i] == NULL){
			destroyBoard(game_board);
			return NULL;
		}
	}

	/*initializing the blocks 3D array + handling memory allocation failures.*/
	blocks_in_a_row = row;
	blocks_in_a_column = column;
	game_board->blocks = (int***)malloc(blocks_in_a_row * sizeof(int**));
	if(game_board->blocks == NULL){
		destroyBoard(game_board);
		return NULL;
	}

	for(i=0;i<blocks_in_a_row; ++i){
		game_board->blocks[i] = (int**)malloc(blocks_in_a_column * sizeof(int*));
		if(game_board->blocks[i] == NULL){
			destroyBoard(game_board);
			return NULL;
		}
		for(j=0; j<blocks_in_a_column; ++j){
			game_board->blocks[i][j] = (int*)malloc(N * sizeof(int));
			if(game_board->blocks[i][j] == NULL){
				destroyBoard(game_board);
				return NULL;
			}
		}
	}
	/*initializing the fixed cells array + handling memory allocation failures.*/
	game_board->fixed_cells = (int**)malloc(N * sizeof(int*));
	if(game_board->fixed_cells == NULL){
		destroyBoard(game_board);
		return NULL;
	}

	for(i=0; i<N; ++i){
		game_board->fixed_cells[i] = (int*)malloc(N * sizeof(int));
		if(game_board->fixed_cells[i] == NULL){
			destroyBoard(game_board);
			return NULL;
		}
	}

	/*initializing the erroneous cells array + handling memory allocation failures.*/
	game_board->err_cells = (int**)malloc(N * sizeof(int*));
	if(game_board->err_cells == NULL){
	    destroyBoard(game_board);
        return NULL;
	}

    for(i=0; i<N; ++i){
        game_board->err_cells[i] = (int*)malloc(N * sizeof(int));
        if(game_board->err_cells[i] == NULL){
            destroyBoard(game_board);
            return NULL;
        }
    }

	initializeBoard(game_board);
	return game_board;
}

Sudoku* copyBoard(Sudoku* board){
    int i, N, blocks_in_a_row, blocks_in_a_column;
    Sudoku* copy;
    N = row_size * column_size;
    blocks_in_a_row = row_size;
    blocks_in_a_column = column_size;

	if(board==NULL){
		return NULL;
	}
	copy = createBoard(row_size, column_size);
    copy2DArray(board->actual_board, copy->actual_board, N , N);
    copy2DArray(board->rows, copy->rows, N, N);
    copy2DArray(board->columns, copy->columns, N, N);
	for( i=0; i<blocks_in_a_row; ++i)
        copy2DArray(board->blocks[i], copy->blocks[i], blocks_in_a_column, blocks_in_a_column);
    copy2DArray(board->fixed_cells, copy->fixed_cells, N, N);
    copy2DArray(board->err_cells, copy->err_cells, N, N);
	return copy;

}

void copyCurrentBoard(Sudoku* copyFrom, Sudoku* copyTo){
    int i, N, blocks_in_a_row, blocks_in_a_column;
    if(copyTo==NULL){
        return;
    }

    N = row_size * column_size;
    blocks_in_a_row = row_size;
    blocks_in_a_column = column_size;

    copy2DArray(copyFrom->actual_board, copyTo->actual_board, N , N);
    copy2DArray(copyFrom->rows, copyTo->rows, N, N);
    copy2DArray(copyFrom->columns, copyTo->columns, N, N);
    for( i=0; i<blocks_in_a_row; ++i)
        copy2DArray(copyFrom->blocks[i], copyTo->blocks[i], blocks_in_a_column, blocks_in_a_column);
    copy2DArray(copyFrom->fixed_cells, copyTo->fixed_cells, N, N);
    copy2DArray(copyFrom->err_cells, copyTo->err_cells, N, N);
    return;

}

void freeMoves(UndoRedoMove* move){
    UndoRedoMove* next;
    Move* inner_move;
    Move* next_inner_move;

    while(move != NULL){
        next = move->next_move;
        inner_move = move->current_move;
        while(inner_move != NULL){
            next_inner_move = inner_move->next;
            free(inner_move);
            inner_move = next_inner_move;
        }
        free(move);
        move = next;
    }
}

int set(Sudoku* board, int column, int row, int value, int isMove){
    int current_value, blocks_in_a_row, blocks_in_a_column, N;
    Move* current;

    current = current_move->current_move;
    blocks_in_a_row = row_size;
    blocks_in_a_column = column_size;
    N = row_size*column_size;

    /*if the current move already exists then create new one and move to it*/
    if(current->row != 0 && isMove){
        current->next = (Move*) malloc(sizeof(Move));
        if(current->next == NULL){
            perror("Memory allocation error.\n");
            exit(EXIT_FAILURE);
        }
        initializeInnerMove(current->next);
        current->next->prev = current;
        current = current->next;
    }
	if(board==NULL)
		return 1;
	if(board->fixed_cells[row-1][column-1] == 1){
		return 2;
	}

	current_value = get(board, column, row);
	if(current_value != 0 && current_value < N){
		board->rows[row-1][current_value-1] -= 1;
		board->columns[column-1][current_value-1] -= 1;
		board->blocks[(column-1)/blocks_in_a_column][(row-1)/blocks_in_a_row][current_value-1] -= 1;
        filled_cells -= 1;

		if(board->err_cells[row-1][column-1]){
            board->err_cells[(row-1)][(column-1)] = 0;
            err_board -= 1;
		}

	}

	/*insert the new value and record the move*/
	board->actual_board[row-1][column-1] = value;

	if(isMove){
        current->row = row;
        current->column = column;
        current->from = current_value;
        current->to = value;
        current_move->current_move = current;
	}

	if(value != 0){
	    /*check if the value is valid before entering it, else it will always be invalid*/
        if(isValid(board, column, row, value) == 0){
            board->err_cells[row-1][column-1] = 1;
            err_board += 1;
        }

		board->rows[row-1][value-1] += 1;
		board->columns[column-1][value-1] += 1;
		board->blocks[(column-1)/blocks_in_a_column][(row-1)/blocks_in_a_row][value-1] += 1;
        filled_cells += 1;
	}
    return 0;
}

int get(Sudoku* board, int column, int row){
	return board->actual_board[row-1][column-1];
}

int isValid(Sudoku* board, int column, int row, int value){
    int blocks_in_a_row, blocks_in_a_column;
    blocks_in_a_row = row_size;
    blocks_in_a_column = column_size;

	if(board->rows[row-1][value-1] == 1)
		return 0;
	if(board->columns[column-1][value-1] == 1)
		return 0;
	if(board->blocks[(column-1)/blocks_in_a_column][(row-1)/blocks_in_a_row][value-1] == 1)
		return 0;
	return 1;
}

void destroyBoard(Sudoku* board){
    int k, blocks_in_a_row, blocks_in_a_column, N;
    N = row_size * column_size;
    blocks_in_a_row = row_size;
    blocks_in_a_column = column_size;

    free2DArray(board->err_cells, row_size);
    free2DArray(board->fixed_cells, row_size);
	for(k=0; k<blocks_in_a_row; ++k){
        free2DArray(board->blocks[k], blocks_in_a_column);
	}
	free(board->blocks);
	board->blocks = NULL;

    free2DArray(board->columns, N);
    free2DArray(board->rows, N);
    free2DArray(board->actual_board, N);
	free(board);
	board = NULL;
}

void printBoard(Sudoku* board){
    int i,j,k, current_row=0;
    int N, n, m = 0;
	if(board == NULL){
		/*printf("Nothing to print");*/
		return;
	}

	N = row_size * column_size;
	n = column_size;
	m = row_size;

	for(i=0; i<N+n+1; ++i){
		if(i%(m+1) == 0){
		    for(k=0; k<(4*N+m+1); ++k){
                putchar('-');
		    }
			printf("\n");
			continue;
		}
		putchar('|');
		for(j=0;j<N;++j){
            if(board->actual_board[current_row][j] == 0)
                printf("  ");
            else
                printf(" %d", board->actual_board[current_row][j]);

			if(board->fixed_cells[current_row][j] == 1)
				printf(". ");
			else if((game_mode==3 && mark_errors==1 && board->err_cells[current_row][j]==1) || (game_mode==2 && board->err_cells[current_row][j]==1))
			    printf("* ");
			else
                printf("  ");

			if(j%(n) == n-1) {
                putchar('|');
            }
		}
		printf("\n");
		current_row = current_row +1;
	}

}

/*function for freeing memory occupied by 2D arrays*/
void free2DArray(int **arr, int size_of_arr) {
    int i;
	if(arr == NULL)
	    return;
	for( i=0; i<size_of_arr; ++i){
		if(arr[i] != NULL){
			free(arr[i]);
			arr[i] = NULL;
		}
	}

	free(arr);
	arr = NULL;
}

void copy2DArray(int **A, int **B, int size_of_A, int size_of_B) { /*copy array A to B*/
    int i,j;/* size_a,size_b, size_a_i, size_b_i;*/
	if(size_of_A != size_of_B){
		/*printf("Error copying arrays, They're not in the same size.");*/
		return;
	}
	for(i=0; i<size_of_A; ++i){
		for(j=0;j<row_size;++j)
			B[i][j] = A[i][j];
	}
}

void initialize2DArray(int** arr){
    int i,j, N;
    N = row_size * column_size;

	for( i=0; i<N; ++i){
		for( j=0; j<N; ++j){
			arr[i][j] = 0;
		}
	}
}

void initializeBoard(Sudoku* board){
    int i,j,k,N,blocks_in_a_row, blocks_in_a_column;
    N = row_size * column_size;
    blocks_in_a_row = row_size;
    blocks_in_a_column = column_size;

    initialize2DArray(board->actual_board);
	initialize2DArray(board->columns);
	initialize2DArray(board->rows);
	initialize2DArray(board->fixed_cells);
	for(i=0; i<blocks_in_a_row; ++i){
		for( j=0; j<blocks_in_a_column; ++j){
			for( k=0; k<N; ++k){
				board->blocks[i][j][k] = 0;
			}
		}
	}
}

int redoMove(Sudoku* board){
   Move* current;
   int valid;

   current = current_move->current_move;
   valid = 0;

   while(current != NULL){
       valid = set(board, current->column, current->row, current->to, 0);
       if(valid)
           return -1;
       printf("Cell at row %d, column %d changed to: %d\n", current->row, current->column, current->to);
       current = current->prev;
   }

   return 0;

}

int undoMove(Sudoku* board){
    Move* current;
    int valid;

    valid = 0;
    current = current_move->current_move;

    while(current != NULL){
        valid = set(board, current->column, current->row, current->from, 0);
        if(valid)
            return -1;
        printf("Cell at row %d, column %d changed to: %d\n", current->row, current->column, current->from);
        current = current->prev;
    }

    /*Move the undo list one move back*/
    current_move = current_move->prev_move;
    return 0;

}

void reset(Sudoku* board){
    while(current_move->prev_move != NULL)
        undoMove(board);
}

void saveBoard(Sudoku* board, char* file){
    FILE *fp;
    int i, j, N;
    N = row_size * column_size;

    if(game_mode == 2 && (err_board > 0 /*|| validateILP() == 0 */)){
        printf("Erroneous boards can not be saved on edit mode.\n");
        return;
    }


    /*
    path = malloc(sizeof(char) *(4 + sizeof(*file)));
    if(path == NULL){
        perror("Error while opening the file.\n");
        exit(EXIT_FAILURE);
    }
     */

    fp = fopen(file, "w");

    if(fp == NULL || !fp){
        perror("Error while opening the file.\n");
        exit(EXIT_FAILURE);
    }

    fprintf(fp, "%d %d\n", row_size, column_size);

    for(i=0; i<N; ++i){
        for(j=0; j<N; ++j){
            fprintf(fp, "%d", board->actual_board[i][j]);
            if(board->actual_board[i][j] && (game_mode == 2 || board->fixed_cells[i][j] == 1))
                fprintf(fp, ".");
            fprintf(fp, " ");
        }
        fprintf(fp, "\n");
    }

    fclose(fp);
}

Sudoku* loadBoard(char* file){
    FILE *fp;
    char ch;
    int row, column, N, m, n, int_ch, curr_val;
    Sudoku* loadedBoard;
    row = 1;
    column = 0;
    m=0;
    n=0;
    curr_val=-1;

    loadedBoard = NULL;


    fp = fopen(file, "r");
    if(fp == NULL){
        perror("Error while opening the file.\n");
        exit(EXIT_FAILURE);
    }
    /*get initial board sizes*/
    while((ch = fgetc(fp)) != '\n' && ch != EOF){
        if(ch != ' ' && m == 0)
            m = ch - 48;
        else if(ch != ' ' && n == 0)
            n = ch - 48;
    }

    /*Change the sizes of the board based on the input*/
    setRowSize(m);
    setColumnSize(n);
    N = m * n;
    loadedBoard = createBoard(m, n);

    /* fill the board*/
    while((ch = fgetc(fp)) != EOF){
        if(ch == ' ' && curr_val == -1)
            continue;

        if(ch == '\n'){
            if(column < N ){
                printf("The file contains a line which is too short\n");
                return NULL;
            }

            if(curr_val > N){
                printf("The file contains an invalid value\n");
                return NULL;
            }
            if(curr_val != -1)
                set(loadedBoard, column, row, curr_val, 0);
            row += 1;
            column = 0;
            continue;
        }

        if(ch == ' ' && curr_val != -1){
            set(loadedBoard, column, row, curr_val, 0);
            curr_val = -1;
        }

        else if(ch == '.' && curr_val != -1){
            set(loadedBoard, column, row, curr_val, 0);
            loadedBoard->fixed_cells[row-1][column-1] = 1;
            curr_val = -1;
        }

        else{
            int_ch = ch - 48;
            if(int_ch < 0 || int_ch > N){
                printf("The file contains an invalid character\n");
                return NULL;
            }
            if(int_ch >= 0 && int_ch <= N){
                if(curr_val == -1) {
                    column += 1;
                    curr_val = int_ch;
                }
                else{
                    curr_val = curr_val*10 + int_ch;
                    if(curr_val > N){
                        printf("The file contains an invalid value\n");
                        return NULL;
                    }
                }

            }
        }
    }

    fclose(fp);
    return loadedBoard;
}

void freeAutoFill(AutoFill* autoFill){
    AutoFill* next;
    while(autoFill != NULL){
        next = autoFill->next;
        free(autoFill);
        autoFill = next;
    }
}

void autofillBoard(Sudoku* board){
    int i, j, k, N, possible_moves, autofill_value;
    AutoFill* autofill_moves;
    AutoFill* curr_move;

    possible_moves = 0;
    autofill_value = 0;
    N = row_size * column_size;
    autofill_moves = (AutoFill*) malloc(sizeof(AutoFill));
    curr_move = autofill_moves;
    curr_move->row = 0;
    curr_move->column = 0;

    if(err_board > 0)
        return;

    /*create linked list of autofill moves*/
    for(i=1; i<=N; ++i){
        for(j=1; j<=N; ++j){
            possible_moves = 0;
            for(k=1; k<=N; ++k){
                if(isValid(board, j, i, k)){
                    if(possible_moves == 0)
                        autofill_value = k;
                    possible_moves += 1;
                }
            }

            if(possible_moves == 1){
                /*Check if move already exists. If so, create new one and move to it*/
                if(curr_move->row != 0){
                    curr_move->next = (AutoFill*) malloc(sizeof(AutoFill));
                    if(curr_move->next == NULL){
                        exit(EXIT_FAILURE);
                    }
                    curr_move = curr_move->next;
                }
                curr_move->row = i;
                curr_move->column = j;
                curr_move->value = autofill_value;
                curr_move->next = NULL;

            }
        }
    }

    /*execute all moves*/
    curr_move = autofill_moves;
    while(curr_move != NULL){
        set(board, curr_move->column, curr_move->row, curr_move->value, 1);
        printf("Cell in row %d, column %d changed to: %d\n", curr_move->row, curr_move->column, curr_move->value);
        curr_move = curr_move->next;
    }

}
/****************/

void setGameMode(int mode){
    game_mode = mode;
    if(game_mode == 1)
        toDefault();
}

int getGameMode(){
    return game_mode;
}

void setMarkErrors(int mode){
    mark_errors = mode;
}

int getMarkErrors(){
    return mark_errors;
}

void setRowSize(int size){
    row_size = size;
}

int getRowSize(){
    return row_size;
}

void setColumnSize(int size){
    column_size = size;
}

int getColumnSize(){
    return column_size;
}

int getErrBoard(){
    return err_board;
}

int getFilledCells(){
    return filled_cells;
}

int advanceMove(){
    UndoRedoMove* clearRedo;
    UndoRedoMove* prev;

    /*clear next moves*/
    if(current_move->next_move != NULL){
        clearRedo = current_move->next_move;
        freeMoves(clearRedo);
    }

    /*initialize the new move*/
    current_move->next_move = (UndoRedoMove*) malloc(sizeof(UndoRedoMove));
    if(current_move->next_move == NULL)
        return -1;

    initializeMove(current_move->next_move);
    prev = current_move;
    current_move = current_move->next_move;
    current_move->prev_move = prev;
    return 0;
}

int prevMove(){
    if(current_move->prev_move == NULL)
        return -1;

    return 0;
}

int nextMove(){
    if(current_move->next_move == NULL)
        return -1;

    current_move = current_move->next_move;
    return 0;
}

int isFixed(Sudoku* board, int column, int row){
    return board->fixed_cells[row-1][column-1];
}

void initializeInnerMove(Move* move){
    move->from = 0;
    move->to = 0;
    move->column = 0;
    move->row = 0;
    move->next = NULL;
    move->prev = NULL;
}

void initializeMove(UndoRedoMove* move){
    move->next_move = NULL;
    move->prev_move = NULL;
    move->current_move = (Move*) malloc(sizeof(Move));
    if(current_move->current_move == NULL){
        perror("Memory allocation error.\n");
        exit(EXIT_FAILURE);
    }
    initializeInnerMove(move->current_move);

}

UndoRedoMove* getFirstMove(){
    return current_move;
}
