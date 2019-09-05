
#include "GRB.h"

int checkAllocatedMem(int *ind, double *val, double *obj, char *vtype, int *chs_cells, double *cellopts);
int freeAllocatedMem(int *ind, double *val, double *obj, char *vtype, int *chosen_cells, double *cellopts);
int checkNumOfVars      (Sudoku *board);
int chooseRandomByWeight(double *cellopts, int count, int N);

/*
 * perform LP and ILP.
 * return value 2 means run-time error.
 */
int GurobiSolution(Sudoku *board, Sudoku *solvedBoard, int is_integer_prog, float threshold, cmd command) {
    /* ------------------------------------------- */
    /* --- Field members and Memory allocating --- */
    /* ------------------------------------------- */

    /* --- General field members --- */
    int     i, j, v, k, tx, ty, shiftx, vars;
    int     error  = 0                      ;
    int     row, column, N                  ;
    int*    chosen_cells                    ; /* each 3 cells will represent (value,x,y) */
    /* --- GRB field members --- */
    GRBenv      *env   = NULL     ;
    GRBmodel    *model = NULL     ;
    int         noStatError ;
    int         optimstatus ;
    int*        ind         ;
    double*     val         ;
    double*     obj         ;
    double*     sol         ;
    char*       vtype       ;
    double*     cellopts    ;
    optimstatus = 0              ;
    row 	= getRowSize()   ;
    column 	= getColumnSize();
    N 		= row * column   ;
    noStatError = 0              ;
    /*check how many variables we need*/
    vars = checkNumOfVars(board);
    /* --- allocate memory --- */
    ind          = (int*   ) malloc((vars > N ? N : vars) * sizeof(int))   ;
    val          = (double*) malloc((vars > N ? N : vars) * sizeof(double));
    obj          = (double*) malloc(vars * sizeof(double));
    sol          = (double*) malloc(vars * sizeof(double));
    vtype        = (char*  ) malloc(vars * sizeof(char))  ;
    chosen_cells = (int*   ) malloc(3*vars*sizeof(int))   ;
    cellopts     = (double*) malloc(2*N   *sizeof(int))   ;
    if (checkAllocatedMem(ind, val, obj, vtype, chosen_cells, cellopts))
        return 2;
    /* --- initiate GRB field members --- */    
    error = GRBloadenv(&env, "sudoku.log");
    if (error) goto QUIT;
      error = GRBsetintparam(env, GRB_INT_PAR_LOGTOCONSOLE, 0);
    if (error) goto QUIT;
    error = GRBnewmodel(env, &model, "sudoku", 0, NULL, NULL, NULL, NULL, NULL);
    if (error) goto QUIT;
    /* ----------------------------------------- */
    /* ---- enter objective function coeffs ---- */
    /* ----------------------------------------- */
    k=0;
/*printf("creating model\n");*/
    for (i=1; i<=N; i++){
        for (j=1; j<=N; j++){
            for (v=1; v<=N; v++){
                if (isFixed(board, i, j) || !isValid(board, i, j, v) || get(board, i, j) != 0) continue;
/*printf("x = %d. y = %d. v = %d.\n",i,j,v);*/
                vtype[k] = (is_integer_prog ? GRB_BINARY : GRB_CONTINUOUS);
                chosen_cells[3*(k)  ] = v; /*z*/
                chosen_cells[3*(k)+1] = i; /*x*/
                chosen_cells[3*(k)+2] = j; /*y*/
                obj[k] = (is_integer_prog ? 1 : (rand()%(N*N))); /* all coeffs should be one for ILP*/
                k++;
            }
        }
    }

    error = GRBaddvars(model, vars, 0, NULL, NULL, NULL, obj, NULL, NULL, vtype, NULL); /*change left 0 to vars*/
    if (error) goto QUIT;
    error = GRBsetintattr(model, GRB_INT_ATTR_MODELSENSE, GRB_MAXIMIZE);
    if (error) goto QUIT;
    error = GRBupdatemodel(model);
    if (error) goto QUIT;
    /* ------------------------- */
    /* --- enter constraints --- */
    /* ------------------------- */

    /*each cell gets a value*/
    for (i=0; i<vars; i++){
        k   = 0;
        tx  = chosen_cells[3*i +1];
        ty  = chosen_cells[3*i +2];
        while (chosen_cells[3*i + 1] == tx && chosen_cells[3*i + 2] == ty) {
            ind[k] = i;
            val[k] = 1.0;
            k++;
            i++;
        }
	i--;
        error = GRBaddconstr(model, k, ind, val, GRB_EQUAL, 1, NULL);
        if (error) goto QUIT;
    }
    /*each value appears once in a column*/
    for (v=1; v<=N; v++){
        i=0;
        while(1){
            k=0;
            tx = chosen_cells[3*i +1];

                while (chosen_cells[3*i + 1] == tx) {
                    if (chosen_cells[3*i] == v){
                        ind[k] = i  ;
                        val[k] = 1.0;
                        k++;
                    }
                    i++;
                    if (i == vars) break;
                }
            error = GRBaddconstr(model, k, ind, val, GRB_EQUAL, 1, NULL);
            if (error) goto QUIT;
	    if (i == vars) break;
        }
    }
    /*each value appears once in a row*/
    for (v=1; v<=N; v++){
        for (j=1; j<=N; j++){
            ty = j;
            k=0;
            for (i=0; i<vars; i++){
                if (chosen_cells[3*i + 2] == ty && chosen_cells[3*i] == v){
                    ind[k] = i  ;
                    val[k] = 1.0;
                    k++;
                }
            }
            error = GRBaddconstr(model, k, ind, val, GRB_EQUAL, 1, NULL);
            if (error) goto QUIT;
        }
    }
    /*each value appears once in a block*/
    for (v=1; v<=N; v++){
        tx     = 0;
        ty     = 0;
        for (i=0; i<N; i++){
            k=0;
            for (j=0; j<vars; j++){
                if((chosen_cells[3*j+1] >= column*tx+1 && chosen_cells[3*j+1] < column*(tx+1)+1) &&
                   (chosen_cells[3*j+2] >= row   *ty+1 && chosen_cells[3*j+2] < row   *(ty+1)+1) &&
                   (chosen_cells[3*j] == v)){
                    ind[k] = j  ;
                    val[k] = 1.0;
                    k++;
                }
            }
            error = GRBaddconstr(model, k, ind, val, GRB_EQUAL, 1, NULL);
            if (error) goto QUIT;
	    shiftx = ((tx == column-1) ? 0 : 1);
            if (shiftx) tx++;
            else {       
               ty++;
               tx=0;
	    }
        }
    }
    /* ------------------------------------------- */
    /* --- Optimize model and capture solution --- */
    /* ------------------------------------------- */
    error = GRBoptimize(model);
    if (error) goto QUIT;
 /*   error = GRBwrite(model, "sudoku.lp");
    if (error) goto QUIT; */
    error = GRBgetintattr(model, GRB_INT_ATTR_STATUS, &optimstatus);
    if (error) goto QUIT;
    else noStatError = 1;
    if(optimstatus != GRB_OPTIMAL) goto QUIT;
    error = GRBgetdblattrarray(model, GRB_DBL_ATTR_X, 0, vars, sol); 
    if (error) goto QUIT; 
    /* -------------------------------------------- */
    /* --- perform actions dependant on command --- */
    /* -------------------------------------------- */
    if (command.name == e_guess) {
        for (i=0; i<vars; i++){
            v  = 0;
            k  = 0;
            tx = chosen_cells[3*i+1];
            ty = chosen_cells[3*i+2];
            while (chosen_cells[3 * (i) + 1] == tx && chosen_cells[3 * (i) + 2] == ty){
/*error = GRBgetdblattrelement(model, GRB_DBL_ATTR_X, i, &sol[i]);
if (error) goto QUIT;*/
                if(sol[i] > threshold){
                    if(isValid(board,tx,ty,chosen_cells[3*i])) {
                        cellopts[v] = sol[i]; /* chance */
                        k += (int) (cellopts[v] * 100.0);
                        v++;
                        cellopts[v] = (double) chosen_cells[3*i]; /* x coordinate */
                        v++; 
                    }
                }
                i++;
            }
            if (v>0){
                j = chooseRandomByWeight(cellopts, k, v);
                set(board, tx, ty, j,1);
            }
            i--;
        }
    }
    else if (command.name == e_guess_hint && optimstatus == GRB_OPTIMAL){
	k=0;
        for(i=0; i<vars; i++){
         if (chosen_cells[3 * i + 1] == command.x && chosen_cells[3 * i + 2] == command.y) {
		k++;
/*error = GRBgetdblattrelement(model, GRB_DBL_ATTR_X, i, &sol[i]);
if (error) goto QUIT;*/
            printf("For the possible value: %d, the chances are: %f\n", chosen_cells[i], sol[i]);
          } 
	if (k==N) break;
	}
    }
    else if (command.name == e_hint && optimstatus == GRB_OPTIMAL){
        tx = command.x;
        ty = command.y;
        i=0;
        k=0;
        printf("For cell (%d, %d):\n",tx, ty);
        while(chosen_cells[3*(i)+1] != tx || chosen_cells[3*(i)+2] != ty) i++;
        do{
            if (chosen_cells[3*i] > 0){
/*error = GRBgetdblattrelement(model, GRB_DBL_ATTR_X, i, &sol[i]);
if (error) goto QUIT;*/

                printf("The chances for value %d are %f.\n",chosen_cells[3*i], sol[i]);
                k++;
            }
	i++;
	if (i == vars-1) break;
        } while(chosen_cells[3*(i)+1] == tx && chosen_cells[3*(i)+2] == ty);
        if (!k) printf("No value found with odds higher than the threshold. Try with a different threshold\n");
    }
    else if(command.name == e_generate && optimstatus == GRB_OPTIMAL ){
        /*solve the board (use the solvedBoard)*/
	 for (i=0; i<vars; i++){
/*error = GRBgetdblattrelement(model, GRB_DBL_ATTR_X, i, &sol[i]);
if (error) goto QUIT;*/
            if(sol[i] > 0.5){
                v =  chosen_cells[i];
                tx = chosen_cells[3*i +1];
                ty = chosen_cells[3*i +2];
                set(solvedBoard, tx, ty, v, 1);
            }
        }
    }
    /* -------------------------------------------- */
    /* --- free all allocated memories and exit --- */
    /* -------------------------------------------- */
    QUIT:
    if (error) {
        printf("ERROR: %s\n", GRBgeterrormsg(env));
    }
    GRBfreemodel(model);
    GRBfreeenv(env);
    freeAllocatedMem(ind, val, obj, vtype, chosen_cells, cellopts);
    if (noStatError && optimstatus == GRB_INF_OR_UNBD) return 2;
    if (error) return 1;
    return 0;
}

int freeAllocatedMem(int *ind, double *val, double *obj, char *vtype, int *chosen_cells, double *cellopts) {
    if (ind         != NULL) free(ind)          ;
    if (val         != NULL) free(val)          ;
    if (obj         != NULL) free(obj)          ;
    if (vtype       != NULL) free(vtype)        ;
    if (chosen_cells!= NULL) free(chosen_cells) ;
    if (cellopts    != NULL) free(cellopts)     ;
    return 0;
}
int checkAllocatedMem(int *ind, double *val, double *obj, char *vtype, int *chs_cells, double *cellopts) {
    int mem_error = 0;
    if (ind == NULL || val == NULL || obj == NULL || vtype == NULL || chs_cells == NULL || cellopts == NULL){
        mem_error = 1;
    }
    if (mem_error){
        printf(" CRITICAL: memory allocating failed... exiting\n");
        freeAllocatedMem(ind, val, obj, vtype, chs_cells, cellopts);
    }
    return mem_error;
}
int chooseRandomByWeight(double *cellopts, int count, int N){
        double rnd;
        int i;
        rnd = rand()%count;
        for (i=0; i<N; i++){
            if (rnd < cellopts[2*i])
                return cellopts[2*i];
            rnd -= cellopts[2*i];
        }
        return 0;
}

int checkNumOfVars(Sudoku *board){
    int R , C    	;
    int i,j,v, count, N	;
    R = getRowSize()	;  
    C = getColumnSize()	;
    N = C*R	;
    count = 0	;
    for (i=1; i<=N; i++){
        for (j=1; j<=N; j++){
            for (v=1; v<=N; v++){
                if (!isFixed(board, i, j) && isValid(board, i, j, v))
                    count++;
            }
        }
    }
    return count;
}




