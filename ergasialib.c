#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int randomInt (int max){
    srand(time(NULL));
    return rand()%max;
}

void yellow(){ printf("\033[0;33m"); }

void green(){ printf("\033[0;32m"); }

void cyan(){ printf("\033[0;36m"); }

void red(){ printf("\033[0;31m"); }

void purple(){ printf("\033[0;35m"); }

void blue(){ printf("\033[0;34m"); }

void clear(){ printf("\033[0m"); }