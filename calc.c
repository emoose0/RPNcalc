#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <ctype.h>
#include <string.h>
#include "calc.h"

interpreter* calc;

//TODO: maybe optimise getnum()?

char* expression;
bool error = false;
int c = 0; //index for expression
const int stackmax = 10000;

void errorMsg(char msg[]){
    fprintf(stderr, "%s\n\n", msg);
    c = 0;
    error = true;
}

void initInterpreter(char* contents){
    calc = (interpreter*)malloc(sizeof(interpreter));
    if(calc == NULL){
        errorMsg("ERR: NO MORE MEMORY TO ALLOCATE CALCULATOR");
        exit(-1);
    }
    calc->tokenList = (token*)malloc(sizeof(token));
    if(calc->tokenList == NULL){
        errorMsg("ERR: NO MORE MEMORY TO ALLOCATE TOKENS");
        exit(-1);
    }
    calc->Ti = 0;
    calc->tokenCount = 0;
    calc->result = 0;
    expression = contents;
    c = 0;
    error = false;
}

stack* newStack(){
    stack* Stack = (stack*)malloc(sizeof(stack));
    Stack->i = -1;
    Stack->items = (token*)malloc(sizeof(token)*stackmax);
    Stack->isEmpty = true;

    return Stack;
}

int push(stack* Stack, token T){
    if(Stack->i >= stackmax){
        errorMsg("ERR: STACK FULL, TRY CHANGING STACKMAX");
        return -1;
    }
    Stack->items[++Stack->i] = T;
    Stack->isEmpty = false;

    return 0;
}

token pop(stack* Stack){
    if(Stack->isEmpty){
        token T = {ERROR, 0};
        errorMsg("ERR: STACK EMPTY");
        return T;
    }
    token T = Stack->items[Stack->i--];
    if(Stack->i == -1){
        Stack->isEmpty = true;
    }
    return T;
}

int precedence(tokenType type){
    switch(type){
        case ADD:
        case SUB:
            return 1;
        case MUL:
        case DIV:
            return 2;
        case POW:
        case NEG:
            return 3;
        default:
            return -1;
    }
}

bool isOp(tokenType type){
    if(type == ADD || type == SUB || type == MUL || type == DIV || type == POW || type == NEG){
        return true;
    }
    return false;
}

bool leftAssociativeness(tokenType type){ //will be used for future purposes, when implementing things such as exponents
    if(type == ADD || type == SUB || type == MUL || type == DIV){
        return true;
    }
    return false;
}

int addToken(tokenType type, double value){
    if(calc->tokenCount*sizeof(token) >= sizeof(token)){ //if number of tokens 
        calc->tokenList = realloc(calc->tokenList, (calc->tokenCount+1)*sizeof(token)); //add space for another token
        if(calc->tokenList == NULL){
            errorMsg("ERR: NO MORE MEMORY TO ALLOCATE TOKENS");
            return -1;
        }
    }
    token T;
    T.type = type;
    T.value = value;

    calc->tokenList[calc->Ti++] = T;
    calc->tokenCount++; 
    return 0;
}

double getNum(){
    bool decimal = false;
    int dCount = 0; //amount of total digits
    int preDCount = 0; //amount of digits before decimal point
    double value = 0;
    int i = c; //index for number counting
    int x = 0; //this will be used to calculate what power of 10 each digit will be multiplied by to complete the number
    while(true){ //horrid way of calculating place count, god save me
        if(isdigit(expression[i])){
            if(!decimal){
                preDCount++;
            }
            dCount++;
        }else if(expression[i] == '.'){
            if(decimal){
                error = true;
                fprintf(stderr, "ERR: STRAY DECIMAL POINT (.) AT CHAR %d\n\n", i);
                c = 0;
                return -1;
            }
            decimal = true;
        }
        i++;
        if((!isdigit(expression[i]) && expression[i] != '.') || expression[i] == '\0'){
            i--;
            break;
        }
    }
    x = dCount - (preDCount-1);
    while(c <= i){
        double num = 0;
        if(isdigit(expression[c])){
            num = (expression[c]-48) * pow(10, dCount-x);
            value += num;
            x++;
        }
        c++;
    }
    c--;
    return value;
}

RESULT tokenise(){
    while(c < strlen(expression)){
        if(isdigit(expression[c]) || expression[c] == '.'){
            double value = getNum();
            addToken(NUMBER, value);
        }else{
            switch(expression[c]){
                case '+':
                    addToken(ADD, 0);
                    break;
                case '-':
                    if(c != 0 && calc->tokenList[calc->Ti-1].type == NUMBER){ //expressions where - is being used as a minus
                        addToken(SUB, 0);
                        break;
                    }
                    addToken(NEG, 0);
                    break;
                case '*':
                    addToken(MUL, 0);
                    break;
                case '/':
                    addToken(DIV, 0);
                    break;
                case '(':
                    if((calc->Ti != 0) && ((calc->tokenList[calc->Ti-1]).type == NUMBER || calc->tokenList[calc->Ti].type == RPAREN)){
                        addToken(MUL, 0);
                    }
                    addToken(LPAREN, 0);
                    break;
                case ')':
                    addToken(RPAREN, 0);
                    if(isdigit(expression[c+1])){
                        addToken(MUL, 0);
                    }
                    break;
                case '^':
                    addToken(POW, 0);
                    break;
                case ' ':
                    break;
                default:
                    fprintf(stderr, "ERR: UNKNOWN TOKEN (%c) AT CHARACTER NUM: %d\n\n", expression[c], c);
                    c = 0;
                    return TOKEN_ERROR;
                    break;
            }
        }
        c++;
        if(error){
            c = 0;
            return TOKEN_ERROR;
        }
    }
    addToken(END, 0);
    RESULT infixEval = infix();
    if(infixEval != 0 || error){
        c = 0;
        return TOKEN_ERROR;
    }
    return SUCCESS;    
}

RESULT eval(double* res, char* contents){
    initInterpreter(contents);
    RESULT evalRes = tokenise();
    /*printf(" %d ", evalRes);
    printTokens();*/
    if(evalRes == SUCCESS){
        calc->Ti = 0;
        stack* Stack = newStack();
        while(calc->tokenList[calc->Ti].type != END){
            double a, b = 0;
            token T;
            T.type = NUMBER;
            if(calc->tokenList[calc->Ti].type == NUMBER){
                push(Stack, calc->tokenList[calc->Ti]);
            } else{
                if(Stack->isEmpty){
                    errorMsg("ERR: FIRST TOKEN CANT BE OPERATOR");
                    return TOKEN_ERROR;
                }
                if(calc->tokenList[calc->Ti].type == ADD){
                    b = pop(Stack).value;
                    if(Stack->isEmpty){
                        errorMsg("ERR: INVALID EXPRESSION");
                        return TOKEN_ERROR;
                    }
                    a = pop(Stack).value;
                    T.value = a+b;
                    push(Stack, T);
                } else if(calc->tokenList[calc->Ti].type == SUB){
                    b = pop(Stack).value;
                    if(Stack->isEmpty){
                        errorMsg("ERR: INVALID EXPRESSION");
                        return TOKEN_ERROR;
                    } else{ 
                        a = pop(Stack).value;
                        T.value = a-b;
                        push(Stack, T);
                    }
                } else if(calc->tokenList[calc->Ti].type == MUL){
                    b = pop(Stack).value;
                    if(Stack->isEmpty){
                        errorMsg("ERR: INVALID EXPRESSION");
                        return TOKEN_ERROR;
                    }
                    a = pop(Stack).value;
                    T.value = a*b;
                    push(Stack, T);
                } else if(calc->tokenList[calc->Ti].type == DIV){
                    b = pop(Stack).value;
                    if(Stack->isEmpty){
                        errorMsg("ERR: INVALID EXPRESSION");
                        return TOKEN_ERROR;
                    }
                    if(b == 0){
                        error = true;
                        fprintf(stderr, "ERR: CANT DIVIDE BY 0\n\n");
                        return CALC_ERROR;
                    }
                    a = pop(Stack).value;
                    T.value = a/b;
                    push(Stack, T);
                } else if(calc->tokenList[calc->Ti].type == POW){
                        b = pop(Stack).value;
                        if(Stack->isEmpty){
                            errorMsg("ERR: INVALID EXPRESSION");
                            return TOKEN_ERROR;
                        }
                        a = pop(Stack).value;
                        T.value = pow(a, b);
                        push(Stack, T);
                } else if(calc->tokenList[calc->Ti].type == NEG){
                    b = pop(Stack).value;
                    T.value = -b;
                    push(Stack, T);
                }
            }
            calc->Ti++;
        }
        if(Stack->i != 0){ //this happens when expressions such as x*y are written in the form of x(y)
            while(Stack->i != 0){
                double b = pop(Stack).value;
                double a = pop(Stack).value;
                token T = {NUMBER, a*b};
                push(Stack, T);
            }
        }
        *res = pop(Stack).value;
    } else{
        return evalRes;
    }
    c = 0;
    error = false;
    return SUCCESS;
}

int infix(){ //convert token of infix expression to postfix
    stack* Tstack = newStack();
    token* infix = (token*)malloc(calc->tokenCount*sizeof(token));
    int i = 0;
    calc->Ti = 0;
    while(calc->tokenList[calc->Ti].type != END){
        if(calc->tokenList[calc->Ti].type == NUMBER){ //move token to new expression if number
            infix[i++] = calc->tokenList[calc->Ti++]; 
        } else if(isOp(calc->tokenList[calc->Ti].type)){
             //while (token on stack is an operator and not an LPAREN) and (token on stack prec > token in tokenlist prec or (theyre equal and the operator in tokenlist is lassoc))
            while(((isOp(Tstack->items[Tstack->i].type)) && Tstack->items[Tstack->i].type != LPAREN) && (precedence(Tstack->items[Tstack->i].type) > precedence(calc->tokenList[calc->Ti].type) || (precedence(Tstack->items[Tstack->i].type) == precedence(calc->tokenList[calc->Ti].type) && leftAssociativeness(calc->tokenList[calc->Ti].type)))){
                infix[i++] = pop(Tstack);
                if(error){
                    return -1;
                }
            }
            push(Tstack, calc->tokenList[calc->Ti++]);
            if(error){
                return -1;
            }
        } else if(calc->tokenList[calc->Ti].type == LPAREN){
            push(Tstack, calc->tokenList[calc->Ti++]);
            if(error){
                return -1;
            }
        } else if(calc->tokenList[calc->Ti].type == RPAREN){
            while(Tstack->items[Tstack->i].type != LPAREN){
                if(Tstack->isEmpty){
                    errorMsg("ERR: MISMATCHED PARENTHEIS");
                    return -1;
                }
                infix[i++] = pop(Tstack);
            }
            if(Tstack->items[Tstack->i].type != LPAREN){
                errorMsg("ERR: MISMATCHED PARENTHEIS");
                return -1;
            }
            pop(Tstack); //discard LPAREN
            if(error){
                return -1;
            }
            calc->Ti++;
        }
    }
    while(!Tstack->isEmpty){
        if(Tstack->items[Tstack->i].type == LPAREN){
            errorMsg("ERR: MISMATCHED PARENTHEIS");
            return -1;
        }
        infix[i++] = pop(Tstack);
        if(error){
            return -1;
        }
    }
    calc->tokenList = infix;
    calc->tokenCount = i;
    calc->Ti = i;
    addToken(END, 0);
    return 0;
}

void printTokens(){
    for(int i = 0; i < calc->tokenCount; i++){
        printf("%f %d\n", calc->tokenList[i].value, calc->tokenList[i].type);
    }
}

void freeInterpreter(){
    free(calc->tokenList);
    free(calc);
}