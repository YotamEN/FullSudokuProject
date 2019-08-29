
#include "GRB.h"
#include "DS.h"

int Get_LP_Solution(Sudoku *board, int integer_prog, int width, int length){

    /* --- GRB field members --- */
    GRBenv      *env                ;
    GRBmodel    *bmodel             ;
    int         board[width][length]; /* FIXME */
    double      val[width]          ;
    /* --- General field members --- */
    int i, j;

    /* --- --- */
    for (i=0; i<width; i++){
        for(j=0; j<length; j++){
            board[i][j] = (get(board, i,j) - 48);
        }
    }



    return -1;
}

