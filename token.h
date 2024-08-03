#ifndef TOKEN_H
#define TOKEN_H

typedef enum{
    NUMBER,
    ADD, SUB, MUL, DIV, POW,
    NEG,
    LPAREN, RPAREN,
    END,
    ERROR
}tokenType;

typedef struct{
    tokenType type;
    double value;
}token;

#endif