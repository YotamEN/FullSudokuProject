#include "solver.h"

typedef struct backtrack_stack BCStack;

struct backtrack_stack{
    int row;
    int column;
    struct backtrack_stack* prev;
};


Sudoku* solve(Sudoku* board){
    Sudoku* copy_board = copyBoard(board);
	return solveBoard(copy_board, 1, 1, 0);
}

Sudoku* solveBoard(Sudoku* board, int column, int row, int isBacktrack){
    int value_to_set;
    int* backtrack_cell;

    if(board == NULL)
		return NULL;
/*	printBoard(board); */
	/*check if cell is fixed or if the value is valid*/
	if((board->actual_board[row-1][column-1] != 0 || board->fixed_cells[row-1][column-1] == 1) && !isBacktrack){
		if(row == 9 && column == 9)
			return board;
		if(column == 9)
			return solveBoard(board, 1, row+1, 0);
		else
			return solveBoard(board, column+1, row, 0);
	}

	value_to_set = 1;

	if(isBacktrack){
        value_to_set = get(board,column,row) + 1;
        set(board, column, row, 0);
	}

	while(!isValid(board, column, row, value_to_set) && value_to_set <= 9){
		/*if the current value is valid continue to the next cell*/
		value_to_set = value_to_set + 1;
	}
	if(value_to_set > 9){ /*Backtracking required*/
		set(board, column, row, 0); /*clear the cell*/
		backtrack_cell = findBacktrackCell(board, column, row);
		if(backtrack_cell == NULL)
			return NULL;
		return solveBoard(board, backtrack_cell[0], backtrack_cell[1], 1);
	}

	else{/*value is valid, set it and proceed to next cell. Else, stay at the same cell.*/
		set(board, column, row, value_to_set);
		if(row == 9 && column == 9)
			return board;
		if(column == 9)
			return solveBoard(board, 1, row+1, 0);
		else
			return solveBoard(board, column+1, row, 0);
	}
}

int exhaustiveBacktracking(Sudoku* board){
    int solutions, row, column, value_to_set, N;
    BCStack* head, *temp;

    solutions = 0;
    row = 1;
    column = 1;
    N = getRowSize() * getColumnSize();

    if(board == NULL)
        return -1;

    if(getFilledCells() == N*N){
        if(getErrBoard() == 0)
            return 1;
        return 0;
    }


    while(1){
        /*check if cell is fixed*/
        if(board->actual_board[row-1][column-1] != 0 || board->fixed_cells[row][column] == 1){
            if(row == N && column == N){
                solutions += 1;
                if(head == NULL)
                    return solutions;
                else{
                    temp = head->prev;
                    row = head->row;
                    column = head->column;
                    free(head);
                    head = temp;
                    continue;
                }
            }
            if(column == N){
                row += 1;
                column = 1;
            }
            else
                column += 1;
            continue;
        }

        /*Cell is empty, we can test values to input*/
        value_to_set = get(board, column, row) + 1;

        while(!isValid(board, column, row, value_to_set) && value_to_set <= N){
            /*if the current value is valid continue to the next cell*/
            value_to_set = value_to_set + 1;
        }

        if(value_to_set > N){ /*Backtracking required*/
            set(board, column, row, 0); /*clear the cell*/

            if(head == NULL) /*Backtrack from the first empty cell*/
                return solutions;

            row = head->row;
            column = head->column;
            temp = head->prev;
            free(head);
            head = temp;
            continue;
        }

        else{/*value is valid, set it and proceed to the next cell.*/
            set(board, column, row, value_to_set);
            if(row == N && column == N){
                solutions += 1;
            }

            else {
                /*Add current cell to the stack*/
                temp = (BCStack*) malloc(sizeof(BCStack));
                if(temp == NULL){
                    perror("Memory allocation error.\n");
                    exit(EXIT_FAILURE);
                }

                temp->row = row;
                temp->column = column;
                if(head == NULL)
                    head = temp;
                else{
                    temp->prev = head;
                    head = temp;
                }

                /*Move to the next cell*/
                if(column == N){
                    row = row+1;
                    column = 1;
                }

                else{
                    column += 1;
                }
            }
        }


    }
}

Sudoku* generatePuzzle(){
    int row, column;
	Sudoku* board;
    row = getRowSize();
    column = getColumnSize();
	board = createBoard(row, column);
	return solveRandBoard(board, 1, 1);
}

Sudoku* solveRandBoard(Sudoku* board, int column, int row){
    int i = 0;
    int value_to_set = 0;
    int valid_values[9] = {0,0,0,0,0,0,0,0,0};
    int range = 0;
    int* values_to_rand;
    int index = 0;
    Sudoku* solution;
    /*static int steps = 0;*/

	if(board == NULL)
		return NULL;

	if((board->fixed_cells[row-1][column-1]) != 0){
		if(row == 9 && column == 9)
			return board;
		if(column == 9)
			return solveRandBoard(board, 1, row+1);
		else
			return solveRandBoard(board, column+1, row);
	}

	value_to_set = 1;

	/*printf("setting value\n");*/

	range = 0;
	for(i=1;i<=9;++i){
		if(isValid(board, column, row, i)){
			valid_values[i-1] = 1;
			range = range+1;
		}
	}

	values_to_rand = (int*)malloc((range*sizeof(int)));

	while(1){
		index = 0;
		for(i=0;i<9;++i){
			if(valid_values[i] == 1){
				values_to_rand[index] = i+1;
				index = index+1;
			}
		}

		/*
		printf("valid values:\n");

		for(i=0; i<index;++i)
		    printf("%d ", values_to_rand[i]);
		printf("range: %d index: %d\n", range, index);
		 */

		if(index == 1)
			value_to_set = values_to_rand[0];
		else if(index > 1)
			value_to_set = values_to_rand[rand()%(index)];
		else if(index == 0){
			set(board, column, row, 0);
			free(values_to_rand);
			return NULL;
		}

		set(board, column, row, value_to_set);
		if(row == 9 && column == 9)
			return board;
		if(column == 9)
			solution = solveRandBoard(board, 1, row+1);
		else
			solution = solveRandBoard(board, column+1, row);

		if(solution != NULL)
			return solution;
		else
			valid_values[value_to_set-1] = 0;
	}

}

int* findBacktrackCell(Sudoku* board, int column, int row){
	static int backtrack_cell[2] = {0,0};
	/*go backwards at least one cell*/
	if(column == 1){
		column = 9;
		row = row-1;
	}
	else
		column = column-1;

	while(row != 0){
		if(board->fixed_cells[row-1][column-1] == 0){
			backtrack_cell[0] = column;
			backtrack_cell[1] = row;
			return backtrack_cell;
		}
		if(column == 1){
			column = 9;
			row = row-1;
		}
		else
			column = column-1;
	}

	return NULL;
}
