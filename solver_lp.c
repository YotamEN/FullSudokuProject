#include "solver_lp.h"
#include "solver_common.h"
#include "gurobi_c.h"
#include <string.h>

#define GET_SUBSQUARE_INDEX(i,j,rows_in_block,cols_in_block) ((rows_in_block*(i/rows_in_block))+(j/cols_in_block))
#define INVALID_INDEX (-1)

/* Static GUROBI enviroment used for solving every LP/ILP program throughout the program's life */
GRBenv   *env   = NULL;

int initialize_solver_resources()
{
    int error;
    /* Create environment */
    if(env == NULL) {
        error = GRBloadenv(&env, "sudoku.log");
        if (error) {
            env = NULL;
            return 1;
        }
        /* Disable logging to console */
        GRBsetintparam(env, GRB_INT_PAR_LOGTOCONSOLE, 0);
        GRBsetintparam(env, GRB_INT_PAR_OUTPUTFLAG, 0);
    }
    return 0;
}
void free_solver_resources()
{
    if(env != NULL) {
        /* Free environment */
        GRBfreeenv(env);
        env = NULL;
    }
}


/* Represents a constraint about several LP variables, as a linked list*/
typedef struct cnstr_list_node_t
{
    int var_index;
    struct cnstr_list_node_t *next;
} cnstr_list_node;

cnstr_list_node *create_list_node(int var_index)
{
    cnstr_list_node *output;
    output = (cnstr_list_node*)calloc(1, sizeof(cnstr_list_node));
    output->var_index = var_index;
    return output;
}
/* Frees a list node and every node reachable from it*/
void free_list(cnstr_list_node *node)
{
    cnstr_list_node *temp;
    while(node != NULL)
    {
        temp = node->next;
        free(node);
        node = temp;
    }
}

/* Creates indexes & coefficents arrays from constraint list.
 * Parameters: head - head of the constraint list
 *             ind - (output) array to fill with the indexes
 *             val - (output) array to fill with coefficents
 *             num_vars - (output) filled with the number of variables added
 */
void make_constraint_from_list(cnstr_list_node* head,int *ind, double* val, int *num_vars)
{
    cnstr_list_node* curr_node;
    int vars_index_in_equation = 0;

    curr_node = head;

    /* Iterate list and add all variables to the indicators array */
    while(curr_node != NULL)
    {
        ind[vars_index_in_equation] = curr_node->var_index;
        val[vars_index_in_equation] = 1.0;

        vars_index_in_equation++;
        curr_node = curr_node->next;
    }

    /* Number of variables added is the same as the next index in the array*/
    *num_vars = vars_index_in_equation;
}

/* Set constraints matching a sudoku board of a given size
 * Parameters: model - gurobi model
 *             dimension - number of rows/columns/subsquares in board. Also the max allowed digit.
 *             cells_constraints_lists      - 2d array of constraint lists. One for every cell.
 *             rows_constraints_lists       - 2d array of constraint lists. One for every row & value.
 *             cols_constraints_lists       - 2d array of constraint lists. One for every column & value.
 *             subsquares_constraints_lists - 2d array of constraint lists. One for every sub-square & value.
 * Returns: 0 if successfuly added all constraints to the model , non-zero if error occurred*/
int set_lp_constraints(GRBmodel *model,int dimension, cnstr_list_node*** cells_constraints_lists,
                       cnstr_list_node*** rows_constraints_lists,
                       cnstr_list_node*** cols_constraints_lists,
                       cnstr_list_node*** subsquares_constraints_lists)
{
    int                   i,j,v;
    int                   sub_square_index;
    int                  *ind;
    double               *val;
    int                   vars_in_equation = 0;
    int                   constraint_failed;
    int                   rows,cols,num_subsquares,max_digit;

    rows = dimension;
    cols = dimension;
    num_subsquares = dimension;
    max_digit = dimension;

    ind = (int*)calloc(rows,sizeof(int));
    val = (double*)calloc(rows,sizeof(double));

    /* Add constraint for every clear cell - must have exactly 1 value*/
    for(i=0;i<rows;i++)
        for(j=0;j<cols;j++)
        {
            if(cells_constraints_lists[i][j] != NULL)
            {
                make_constraint_from_list(cells_constraints_lists[i][j],ind,val,&vars_in_equation);

                constraint_failed = GRBaddconstr(model, vars_in_equation, ind, val, GRB_EQUAL, 1.0, NULL);
                if (constraint_failed)
                {
                    free(ind);
                    free(val);
                    return constraint_failed;
                }
            }
        }

    /* Add constraint for every row - must have at every missing digit exactly once */
    for(i=0;i<rows;i++)
        for(v=0;v<max_digit;v++)
        {
            if(rows_constraints_lists[i][v] != NULL)
            {
                make_constraint_from_list(rows_constraints_lists[i][v],ind,val,&vars_in_equation);

                constraint_failed = GRBaddconstr(model, vars_in_equation, ind, val, GRB_EQUAL, 1.0, NULL);
                if (constraint_failed)
                {
                    free(ind);
                    free(val);
                    return constraint_failed;
                }
            }
        }

    /* Add constraint for every column - must have at every missing digit exactly once */
    for(j=0;j<cols;j++)
        for(v=0;v<max_digit;v++)
        {
            if(cols_constraints_lists[j][v] != NULL)
            {
                make_constraint_from_list(cols_constraints_lists[j][v],ind,val,&vars_in_equation);

                constraint_failed = GRBaddconstr(model, vars_in_equation, ind, val, GRB_EQUAL, 1.0, NULL);
                if (constraint_failed)
                {
                    free(ind);
                    free(val);
                    return constraint_failed;
                }
            }
        }

    /* Add constraint for every sub square - must have at every missing digit exactly once */
    for(sub_square_index=0;sub_square_index<num_subsquares;sub_square_index++)
        for(v=0;v<max_digit;v++)
        {
            if(subsquares_constraints_lists[sub_square_index][v] != NULL)
            {
                make_constraint_from_list(subsquares_constraints_lists[sub_square_index][v],ind,val,&vars_in_equation);

                constraint_failed = GRBaddconstr(model, vars_in_equation, ind, val, GRB_EQUAL, 1.0, NULL);
                if (constraint_failed)
                {
                    free(ind);
                    free(val);
                    return constraint_failed;
                }
            }
        }

    free(ind);
    free(val);
    return 0;
}

#define MIN_OPTIONS_NOT_INITIALIZED 0

/* Returns the minimal amount of options for a cell with a given subsquare that accepts a given value
 * Parameters: b - Board to check
 *             value - the value to search in the subsquare (0-based)
 *             sub_square_index - index of the subsquare to check
 *             var_indexes - 3d array representing indexes of cells+values that LP variables were created for.
 *                           Will be used to determine the options number of every cell
 * Return the minimal options count as described above
 * */
int get_min_options(board *b,int value,int sub_square_index)
{
    int i;
    int j;
    int v;
    int max_digit;
    int rows_in_block,cols_in_block;
    int first_sub_square_index_row;
    int first_sub_square_index_col;
    int counter;
    int min_counter;

    max_digit = get_max_digit(b);
    rows_in_block = get_rows_in_block_count(b);
    cols_in_block = get_columns_in_block_count(b);

    first_sub_square_index_col = (sub_square_index % rows_in_block) * cols_in_block;
    first_sub_square_index_row = (sub_square_index / rows_in_block) * rows_in_block;

    /* Setting variable to hold the minimum found.
     * Max value for this counter is the amount of all digits */
    min_counter = max_digit;
    for(i=first_sub_square_index_row;i<(first_sub_square_index_row+rows_in_block);i++) {
        for(j=first_sub_square_index_col;j<(first_sub_square_index_col+cols_in_block);j++) {
            /* Check if cell [i,j] allows the requested value*/
            if(check_value_for_cell(b,i+1,j+1,value+1) == SOLVER_CHECK_RESULT_OK)
            {
                /* Count options for the cell [i,j]*/
                counter = 0;
                for (v = 0; v < max_digit; v++) {
                    if(check_value_for_cell(b,i+1,j+1,v+1) == SOLVER_CHECK_RESULT_OK) {
                        counter++;
                    }
                }
                if(counter < min_counter){
                    min_counter = counter;
                }
            }
        }
    }

    return min_counter;
}

/* Returns the coefficient to use for a given variable (row index, column index and value) in a given board
 * This function defines the objective function entirely.
 * The logic is as follows:
 * 1. For ILP, returns 0 for all variable. Hence the objective function will be empty.
 * 2. For LP, Checks if the value is 'very probable' for this cell. If so, returns a random number between 1 and 5
 *                                                                 If it isn't returns 0 (var not included in the obj function)
 * In this context, value "v" is considered 'very probable' for cell "[i,j]" is:
 * Within the sub-square of [i,j] the cell [i,j] has a list of possible values of minimal length
 * and also v is a possible value for this cell.
 * Example: If within a sub-square 4 cells can have the values 1 and 2
 *          and 3 of them can also have the value 3
 *          then 1 is considered 'very probable' for the 4th cell (it's values list is the shortest among those
 *          which contain the value 1).
 * For the "Guess function": Among 20,000 random boards tested (with several different thresholds) using this function
 * to generate  * the objective function generated solvable boards in 89% of the tests.
 * For comparision, When using the Guess option with no objective function  on the same boards
 * resulted in solvable boards in only 75% of the boards.
 *
 * Parameters: b - board to check
 *             lp_mode - whether we are solving an ILP or LP program
 *             i - row index of the cell
 *             j - col index of the cell
 *             value - the value to check for the cell
 *             min_options_count - array of minimal 'possible values' lists length for every value and every subsquare
 * Returns: The coefficient to use for this variable in the objective function.
 */
int get_obj_coeff(board *b,int lp_mode, int i, int j, int value, int** min_options_count)
{
    int sub_square_index;
    int rows_in_block;
    int cols_in_block;
    int max_digit;
    int counter;
    int v;

    if(lp_mode == SOLVER_ALGO_INTEGER_LINEAR_PROG){
        /* No objective function needed for integer LP */
        return 0;
    }

    rows_in_block = get_rows_in_block_count(b);
    cols_in_block = get_columns_in_block_count(b);
    max_digit = get_max_digit(b);

    /* Find the subsquare of the given cell */
    sub_square_index = GET_SUBSQUARE_INDEX(i,j,rows_in_block,cols_in_block);

    /* Checking if the already have the shortest list length stored from a previous checked cell */
    if(min_options_count[value][sub_square_index] == MIN_OPTIONS_NOT_INITIALIZED) {
        /* We don't have the length, calling get_min_options() to find it (and store for later cells) */
        min_options_count[value][sub_square_index] = get_min_options(b,value,sub_square_index);
    }

    /* Count options for the current cell [i,j]*/
    counter = 0;
    for (v = 0; v < max_digit; v++) {
        if(check_value_for_cell(b,i+1,j+1,v+1) == SOLVER_CHECK_RESULT_OK) {
            counter++;
        }
    }
    /* Check if minimum is achieved in the current cell */
    if(counter == min_options_count[value][sub_square_index]){
        /* Minimum options for this value is found in this cell! Adding to objective function with random weight */
        return (rand() % 5) +1;
    }
    /* Not part of the objective... */
    return 0;
}

/* Randomly chooses number between 1 to (arr_size-1) based on given probabilities */
/* Parameters: prob_array - array of probabilites. For each index i, prob_array[i] should
 *                          be a double between 0 and 1 representing it's probability to be choosen
 *             arr_size - size of prob_array
 * Returns: One of the indexes randomally, or -1 on failure
 */
int weighted_random_digit_selection(double *prob_array,int arr_size)
{
    int *instances_counts;
    int *instances;
    int total_instances;
    int derieved_instances_count;
    int i;
    int j;
    int k;
    int choosen_instance_index;
    int choosen_digit;

    /* Getting the amount of instances desired for each digit */
    /* Taking into account 2 positions after the dot*/
    instances_counts = (int*)calloc(arr_size, sizeof(int));

    total_instances = 0;
    for(i=1;i<arr_size;i++) {
        derieved_instances_count = (int)(prob_array[i] * 100);
        instances_counts[i] = derieved_instances_count;
        total_instances += derieved_instances_count;
    }


    /* Placing *instances_counts[i]* times the digit i in the instances array */
    instances = (int*)calloc(total_instances, sizeof(int));
    k = 0;
    for(i=1;i<arr_size;i++) {
        for(j=0;j< instances_counts[i]; j++) {
            instances[k] = i;
            k++;
        }
    }

    if(total_instances != 0) {
        choosen_instance_index = (rand() % total_instances);
        choosen_digit = instances[choosen_instance_index];
    }
    else {
        /* No indexes to choose frome, this is an error */
        choosen_digit = -1;
    }

    free(instances);
    free(instances_counts);

    return choosen_digit;
}

/* Tries to get back the soduko solution form the gurobi model
 * Parameters: model - optimized gurobi module
 *             lp_mode - whether we are using LP or ILP
 *             threshold - threshold for probabilities of values for each cell. Only relevant for LP.
 *             b - original board
 *             output - board to fill with optimization values
 * Returns: 0 if successful and output board filled, 1 otherwise. */
int try_retrieve_solution(GRBmodel *model, int lp_mode, float threshold,  board *b, board *output_board)
{
    int *digits;
    double   *digits_and_probabilities;
    int next_available_index;
    int index;
    int i,j,k,v;
    double probability_for_v;
    int rows,cols;
    int max_digit;
    int error;
    int selected_digit;

    error = 0;

    rows = get_rows_count(b);
    cols = get_columns_count(b);
    max_digit = get_max_digit(b);

    /* Create array of digits indicator */
    digits = (int*)calloc(max_digit+1,sizeof(int));
    /* Create array of digits probabilities */
    digits_and_probabilities = (double*)calloc(max_digit+1,sizeof(double));

    /* Retreiving the solution from the model */
    next_available_index = 0;
    for (i = 0; i < rows; i++) {
        for (j = 0; j < cols; j++) {
            if(get_number(b,i+1,j+1) != 0) {
                /* Cell was provided in the input with a value, copy to output and move on */
                set_number(output_board, i + 1, j + 1, get_number(b,i+1,j+1));
                if (get_fixed(b, i + 1, j + 1) == FIXED_CELL) {
                    set_fixed(output_board, i + 1, j + 1, FIXED_CELL);
                }
                continue;
            }

            /* Get invalid digits according to INPUT board */
            find_available_digits_for_cell(b,i+1,j+1,digits);

            /* Reset probabilities */
            for(k=1;k<=max_digit;k++)
                digits_and_probabilities[k] = 0;

            /* Check every value for this cell */
            for (v = 0; v < max_digit; v++) {
                if(digits[v+1] == DIGIT_VALID) {
                    index = next_available_index;
                    next_available_index++;
                    error |= GRBgetdblattrelement(model, GRB_DBL_ATTR_X, index,
                                                  &probability_for_v);

                    if(lp_mode == SOLVER_ALGO_INTEGER_LINEAR_PROG) {
                        /* FOR ILP - Choose any cell with probablity of 1
                         * (checking against 0.5 because of possible floating point mistakes)
                         */
                        if (!error && probability_for_v > 0.5) {
                            set_number(output_board, i + 1, j + 1, v + 1);
                        }
                    }
                    else {
                        /* FOR LP - Collect values and possibilities but only for VALID cells after choosing
                         * values for previous cells */
                        if(check_value_for_cell(output_board,i+1,j+1,v+1) == SOLVER_CHECK_RESULT_ERROR){
                            /* Invalid value, skipping */
                            continue;
                        }
                        if(probability_for_v > threshold) {
                            digits_and_probabilities[v + 1] = probability_for_v;
                        }
                    }
                }
            }
            /* FOR LP - Collected all values probabilities, time to choose one randomly */
            if (lp_mode == SOLVER_ALGO_LINEAR_PROG) {
                selected_digit = weighted_random_digit_selection(digits_and_probabilities, max_digit+1);
                if(selected_digit != -1) {
                    set_number(output_board, i + 1, j + 1, selected_digit);
                }
            }
        }
    }

    free(digits);
    free(digits_and_probabilities);

    if(error)
        return 1;
    else
        return 0;
}

GRBmodel* make_and_solve_lp_model(board *b,int lp_mode)
{
    GRBmodel *model = NULL;
    int      *digits;
    int       optimstatus;
    int       i, j, v;
    int       error = 0;
    int       temp_error = 0;
    char       var_type;
    double       obj_coeff;
    int max_digit;
    int rows, cols, subsquares;
    int rows_in_block, cols_in_block;
    int next_available_index;
    int any_available_digit;

    /* A 2d array. Indexes are [value][subsquare_indx]. The stored value
     * is the amount of available values for a cell in the subsquare among all
     * cells that can accommodate the given value.
     */
    int **min_options_count;

    int current_index;
    int dimension;
    int sub_square_index;
    cnstr_list_node ***cells_constraints_lists; /* 2d list of pointers */
    cnstr_list_node ***rows_constraints_lists; /* 2d list of pointers */
    cnstr_list_node ***cols_constraints_lists; /* 2d list of pointers */
    cnstr_list_node ***subsquares_constraints_lists; /* 2d list of pointers */
    cnstr_list_node *current_list;


    /* Get sizes from board*/
    rows_in_block = get_rows_in_block_count(b);
    cols_in_block = get_columns_in_block_count(b);
    rows = get_rows_count(b);
    cols = get_columns_count(b);
    max_digit = get_max_digit(b);
    dimension = rows; /* Same value for rows,cols,num of subsquars and max digit*/
    subsquares = dimension;

    /* Allocate all constraints lists used in this function */
    cells_constraints_lists = (cnstr_list_node***)calloc(dimension,sizeof(cnstr_list_node**));
    rows_constraints_lists = (cnstr_list_node***)calloc(dimension,sizeof(cnstr_list_node**));
    cols_constraints_lists = (cnstr_list_node***)calloc(dimension,sizeof(cnstr_list_node**));
    subsquares_constraints_lists = (cnstr_list_node***)calloc(dimension,sizeof(cnstr_list_node**));
    for(i=0;i<dimension;i++) {
        cells_constraints_lists[i] = (cnstr_list_node ** )calloc(dimension, sizeof(cnstr_list_node * ));
        rows_constraints_lists[i] = (cnstr_list_node ** )calloc(dimension, sizeof(cnstr_list_node * ));
        cols_constraints_lists[i] = (cnstr_list_node ** )calloc(dimension, sizeof(cnstr_list_node * ));
        subsquares_constraints_lists[i] = (cnstr_list_node ** )calloc(dimension, sizeof(cnstr_list_node * ));
    }


    min_options_count = (int**)calloc(max_digit,sizeof(int*));
    for(v=0;v<max_digit;v++) {
        min_options_count[v] = (int*)calloc(subsquares,sizeof(int));
        for(i=0;i<subsquares;i++) {
            min_options_count[v][i] = MIN_OPTIONS_NOT_INITIALIZED;
        }
    }


    /* init environment */
    error = initialize_solver_resources();

    /* Create new model */
    if(!error)
        error = GRBnewmodel(env, &model, "sudoku", 0, NULL, NULL, NULL, NULL, NULL );


    digits = (int*)calloc(max_digit+1,sizeof(int));

    /* Create variables: Only creating meaningful (possible) variables
     * Also, while we are creating them we are concatinating them in all relevant 'constraints lists'
     * that we later translate into constraints for the GUROBI model 
     *
     * There's also a check below that every empty cell has at least 1 available value.
     */
    next_available_index = 0;
    for (i = 0; i < rows; i++) {
        for (j = 0; j < cols; j++) {
            if(get_number(b,i+1,j+1) != 0) {
                /* Cell was provided with a value, move on */
                continue;
            }
            /* Found an empty cell we need to fill.*/
            /* Resettubg number of available digits (or numver of craeted variables) for this cell */
            any_available_digit = 0;
            
            /* Get available values for [i+1,j+1]*/
            find_available_digits_for_cell(b,i+1,j+1,digits);

            /* Check every value */
            for (v = 0; v < max_digit; v++) {
                if(digits[v+1] == DIGIT_VALID) {
                    /* Value 'v+1' is valid for the current cell, allocate a variable (index) */
                    current_index = next_available_index;
                    next_available_index++;
                    /* Count this digit as a valid option */
                    any_available_digit++;


                    /*Adding this variable to any fitting constraints: */
                    /* Cell constraint */
                    current_list = cells_constraints_lists[i][j];
                    cells_constraints_lists[i][j] = create_list_node(current_index);
                    if(current_list != NULL)
                    {
                        /* Append previous list if existed*/
                        cells_constraints_lists[i][j]->next = current_list;
                    }
                    /* Row constraint */
                    current_list = rows_constraints_lists[i][v];
                    rows_constraints_lists[i][v] = create_list_node(current_index);
                    if(current_list != NULL)
                    {
                        /* Append previous list if existed*/
                        rows_constraints_lists[i][v]->next = current_list;
                    }
                    /* Col constraint */
                    current_list = cols_constraints_lists[j][v];
                    cols_constraints_lists[j][v] = create_list_node(current_index);
                    if(current_list != NULL)
                    {
                        /* Append previous list if existed*/
                        cols_constraints_lists[j][v]->next = current_list;
                    }
                    /* Sub square constraint */
                    sub_square_index = GET_SUBSQUARE_INDEX(i,j,rows_in_block,cols_in_block);
                    current_list = subsquares_constraints_lists[sub_square_index][v];
                    subsquares_constraints_lists[sub_square_index][v] = create_list_node(current_index);
                    if(current_list != NULL)
                    {
                        /* Append previous list if existed*/
                        subsquares_constraints_lists[sub_square_index][v]->next = current_list;
                    }

                    obj_coeff = get_obj_coeff(b,lp_mode,i,j,v,min_options_count);

                    if (lp_mode == SOLVER_ALGO_INTEGER_LINEAR_PROG) {
                        var_type = GRB_BINARY;
                    } else {
                        /* mode is set to SOLVER_ALGO_LINEAR_PROG */
                        var_type = GRB_CONTINUOUS;
                    }
                    temp_error = GRBaddvar(model,0,NULL,NULL,obj_coeff, 0, 1,var_type, NULL);
                    error |= temp_error;
                }
            }
            /* Make sure at least 1 digit is available for this cell
             * Otherwise the board is unsolvable
             */
            if(any_available_digit == 0) {
                error = 1;
            }
        }
        
    }
    /* Update model - to apply variables addings */
    if(!error)
        error |= GRBupdatemodel(model);

    /* TODO: Remove Write model to 'sudoku.lp' */
    if(!error)
        error |= GRBwrite(model, "sudoku.lp");

    /* Set constraints according to lists generated above */
    if(!error)
        error = set_lp_constraints(model, dimension, cells_constraints_lists,
                                   rows_constraints_lists,
                                   cols_constraints_lists,
                                   subsquares_constraints_lists);


    /* Optimize model */
    if(!error)
        error = GRBoptimize(model);

    /* Get back solution information */
    if(!error)
        error = GRBgetintattr(model, GRB_INT_ATTR_STATUS, &optimstatus);

    if(optimstatus != GRB_OPTIMAL)
        error = 1;


    free(digits);

    for(i=0;i<dimension;i++) {
        for(j=0;j<dimension;j++) {
            free_list(cells_constraints_lists[i][j]);
            free_list(rows_constraints_lists[i][j]);
            free_list(cols_constraints_lists[i][j]);
            free_list(subsquares_constraints_lists[i][j]);
        }
        free(cells_constraints_lists[i]);
        free(rows_constraints_lists[i]);
        free(cols_constraints_lists[i]);
        free(subsquares_constraints_lists[i]);
    }
    free(cells_constraints_lists);
    free(rows_constraints_lists);
    free(cols_constraints_lists);
    free(subsquares_constraints_lists);

    for(v=0;v<max_digit;v++) {
        free(min_options_count[v]);
    }
    free(min_options_count);


    if(error){
        /* Error, free partial model and return NULL*/
        GRBfreemodel(model);
        return NULL;
    }

    /* Success */
    return model;
}

/* A base function for LP/ILP board solving. If solving with LP returns an "approximate solution" (guess)
 * Parameters: b - board to solve
 *             lp_mode - which mode to run, LP or ILP
 *             threshold - Only relevant for LP. Determins the treshold from which values are considered
 *                         a possibility for every cell after solving with LP.
 *             output_board - board where the solution will be writter on success.
 * Returns: Either SOLVER_RESULT_SOLVED or SOLVER_RESULT_UNSOLVABLE according to functions result.
 */
int solve_base_function(board *b,int lp_mode,float threshold, board *output_board)
{
    GRBmodel *model = NULL;
    int       error = 0;

    model = make_and_solve_lp_model(b,lp_mode);
    if(model == NULL) {
        error = 1;
    }

    if(!error) {
        error = try_retrieve_solution(model, lp_mode,threshold, b, output_board);
    }

    if(model != NULL) {
        /* Free model */
        GRBfreemodel(model);
    }



    if(error)
    {
        return SOLVER_RESULT_UNSOLVABLE;
    }
    else
    {
        return SOLVER_RESULT_SOLVED;
    }
}

solver_results* solve(board *b)
{
    solver_results* res;
    board* b_copy;
    board* output;
    int algo_res;

    res = create_solver_results();
    b_copy = create_board(get_rows_in_block_count(b),get_columns_in_block_count(b));
    output = create_board(get_rows_in_block_count(b),get_columns_in_block_count(b));
    copy_board(b,b_copy);

    algo_res = solve_base_function(b_copy,SOLVER_ALGO_INTEGER_LINEAR_PROG,0.5f,output);

    delete_board(b_copy);
    if(algo_res == SOLVER_RESULT_SOLVED)
    {
        res->result_code = SOLVER_RESULT_SOLVED;
        res->solved_board = output;
    }
    else
    {
        res->result_code = SOLVER_RESULT_UNSOLVABLE;
        res->solved_board = NULL;
        delete_board(output);
    }

    return res;
}
