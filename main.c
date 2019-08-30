#include <stdio.h>
#include <stdlib.h>
#include "game.h"
#include "DS.h"
#include "solver.h"
#include "parser.h"

int main(int argc, char *argv[]){
    int seed = 0;
    /*
     * Getting seed from CMD
     * */
    if (argc > 1) {
        if (sscanf(argv[1], "%d", &seed) <= 0) {
            exit(EXIT_MSG2("main"));
        }
        srand(seed);
    }
    else {
        srand(time(NULL));
    }
    game();
    return 0;
}
