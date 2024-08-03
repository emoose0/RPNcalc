#ifndef CALC_H
#define CALC_H
#include <stdbool.h>
#include "token.h"

typedef struct{
    token* tokenList;
    int tokenCount;
    int Ti;
    double result;
}interpreter;

typedef enum{
    SUCCESS, 
    CALC_ERROR, TOKEN_ERROR
}RESULT;

typedef struct stack{
    int stackmax;
    int i; //points to topmost element
    token* items;
    bool isEmpty;
}stack;

void initInterpreter(char* contents);
int infix();
RESULT tokenise();
RESULT eval(double* res, char* contents);
void printTokens();
void freeInterpreter();

#endif