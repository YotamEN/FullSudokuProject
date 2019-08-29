#include "DS.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int game_mode = 1;
int mark_errors = 1;
int row_size = 3;
int column_size = 3;
int err_board = 0;
typedef struct undo_redo_move UndoMove;

struct undo_redo_move{
    struct move* current_move;
    struct undo_redo_move* next_move;
    struct undo_redo_move* prev_move;
};

typedef struct autofill_move AutoFill;

struct autofill_move{
    int row;
    int column;
    int value;
    struct autofill_move* next;
};

UndoMove* first_move = NULL;
UndoMove* current_move = NULL;

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

	for( i=0; i<N; ++i){
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

int set(Sudoku* board, int column, int row, int value){
    int ret=0, current_value, blocks_in_a_row, blocks_in_a_column;
    blocks_in_a_row = row_size;
    blocks_in_a_column = column_size;

    /*
    if((column < 1 || column > row_size) || (row < 1 || row > row_size) || (value < 1 || value > 9)){
        printf("Error: invalid command\n");
        return 0;
    }
     */

	if(board==NULL)
		return 0;
	if(board->fixed_cells[row-1][column-1] == 1){
		return 2;
	}

	current_value = get(board, column, row);
	if(current_value != 0){
		board->rows[row-1][current_value-1] = 0;
		board->columns[column-1][current_value-1] = 0;
		board->blocks[(row-1)/blocks_in_a_row][(column-1)/blocks_in_a_column][current_value-1] = 0;
		board->err_cells[(row-1)][(column-1)] = 0;
		ret--;
	}

	board->actual_board[row-1][column-1] = value;

	if(value != 0){
		board->rows[row-1][value-1] = 1;
		board->columns[column-1][value-1] = 1;
		board->blocks[(row-1)/blocks_in_a_row][(column-1)/blocks_in_a_column][value-1] = 1;

        if(isValid(board, column, row, value) == 0){
            board->err_cells[row-1][column-1] = 1;
            /*return 3;*/
        }

		ret++;
	}
    return ret;
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
	if(board->blocks[(row-1)/blocks_in_a_row][(column-1)/blocks_in_a_column][value-1] == 1)
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

	for(i=0; i<n+N+1; ++i){
		if(i%(m+1) == 0){
		    for(k=0; k<(4*N+m+1); ++k){
                putchar('-');
		    }
			printf("\n");
			continue;
		}
		putchar('|');
		putchar(' ');
		for(j=0;j<N;++j){
            if(board->actual_board[current_row][j] == 0)
                printf("  ");
            else
                printf("%d", board->actual_board[current_row][j]);
			if(board->fixed_cells[current_row][j] == 1)
				putchar('.');
			else if((game_mode==3 && mark_errors==1 && board->err_cells[current_row][j]==1) || (game_mode==2 && board->err_cells[current_row][j]==1))
			    putchar('*');
			else
				putchar(' ');

			if(j%(n) == n-1) {
                putchar('|');
                /*We night need to change this */
                if(j+1 != row_size)
                    putchar(' ');
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
    int i,j;
	for( i=0; i<row_size; ++i){
		for( j=0; j<row_size; ++j){
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

/*need to define with Yotam*/
void reverseMove(){

}

void addMove(){

}

void redoMove(){

}

void undoMove(){

}

void reset(){
    while(current_move->prev_move != NULL)
        undoMove();
}

void saveBoard(Sudoku* board, char* file){
    File *fp;
    int i, j, N;
    N = row_size * column_size;

    if(game_mode == 2 && (err_board > 0 /*|| validateILP() == 0 */))
        return;

    fp = fopen(file, "w");

    if(fp == NULL){
        perror("Error while opening the file.\n");
        exit(EXIT_FAILURE);
    }

    fprintf(fp, "%d %d\n", row_size, column_size);

    for(i=0; i<N; ++i){
        for(j=0; j<N; ++j){
            fprintf(fp, "%d", board->actual_board[i][j]);
            if(game_mode == 2 || board->fixed_cells[i][j] == 1)
                fprintf(fp, ".");
            fprintf(fp, " ");
        }
        fprintf(fp, "\n");
    }

    fclose(fp);
}

Sudoku* loadBoard(char* file){
    File *fp;
    char ch;
    int row = 1, column = 1, N, m, n, int_ch, curr_val = -1;
    Sudoku* loadedBoard = NULL;

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

    loadedBoard = createBoard(m, n);

    /* fill the board*/
    while((ch = fgetc(fp)) != EOF){
        if(ch == ' ' && curr_val == -1)
            continue;

        if(ch == '\n'){
            if(curr_val != -1)
                set(loadedBoard, column, row, curr_val);
            row += 1;
            column = 1;
            continue;
        }

        if(ch == ' ' && curr_val != -1){
            set(loadedBoard, column, row, curr_val);
            curr_val = -1;
        }

        else if(ch == '.' && curr_val != -1){
            set(loadedBoard, column, row, curr_val);
            loadedBoard->fixed_cells[row][column] = 1;
            curr_val = -1;
        }

        else{
            int_ch = ch - 48;
            if(int_ch >= 0 && int_ch <= 9){
                if(curr_val == -1) {
                    column += 1;
                    curr_val = int_ch;
                }
                else
                    curr_val = curr_val*10 + int_ch;
            }
        }
    }

    return loadedBoard;
}


void autofillBoard(Sudoku* board){
    if(err_board > 0)
        return;


}
/****************/

void setGameMode(int mode){
    game_mode = mode;
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
