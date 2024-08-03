#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "calc.h"

int main(int argc, char* argv[]){
    int size = 10;
    char* expression = (char*)malloc(size);  
    printf("type expressions here (q to quit):\n");

    while(true){
        char c;
        int count = 0;
        size = 10;
        expression = (char*)malloc(size);
        double result = 0;
        printf("> ");
        while((c=fgetc(stdin)) != '\n'){

            if(count > size-1){
                size += 10;
                expression = realloc(expression, size);
                if(expression == NULL){
                    fprintf(stderr, "ERR: NOT ENOUGH MEMORY");
                    return -1;
                }
            }
            expression[count++] = c;
        }
        expression = realloc(expression, size+1);
        expression[count++] = '\0';

        expression = realloc(expression, strlen(expression)+1);

        if(expression == NULL){
            fprintf(stderr, "ERR: NOT ENOUGH MEMORY");
            return -1;
        }

        if(strcmp(expression, "q") == 0){
            break;
        }


        RESULT final = eval(&result, expression);
        if(final == SUCCESS){
            printf("%f\n\n", result);
        }
        free(expression);
        expression = NULL;
        freeInterpreter();
    }
    free(expression);
    expression = NULL;

    return 0;
}