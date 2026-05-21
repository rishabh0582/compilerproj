#ifndef LEXERDEF_H
#define LEXERDEF_H

#define MAX_LEXEME_LEN 100

typedef enum {

    TK_WITH,
    TK_PARAMETERS,
    TK_END,
    TK_WHILE,
    TK_UNION,
    TK_ENDUNION,
    TK_DEFINETYPE,
    TK_AS,
    TK_TYPE,
    TK_MAIN,
    TK_GLOBAL,
    TK_PARAMETER,
    TK_LIST,
    TK_INPUT,
    TK_OUTPUT,
    TK_INT,
    TK_REAL,
    TK_ENDWHILE,
    TK_IF,
    TK_THEN,
    TK_ENDIF,
    TK_READ,
    TK_WRITE,
    TK_RETURN,
    TK_CALL,
    TK_RECORD,
    TK_ENDRECORD,
    TK_ELSE,

    TK_ASSIGNOP,
    TK_PLUS,
    TK_MINUS,
    TK_MUL,
    TK_DIV,
    TK_AND,
    TK_OR,
    TK_NOT,

    TK_LT,
    TK_LE,
    TK_EQ,
    TK_GT,
    TK_GE,
    TK_NE,

    TK_SQL,
    TK_SQR,
    TK_OP,
    TK_CL,
    TK_COMMA,
    TK_SEM,
    TK_COLON,
    TK_DOT,

    TK_ID,
    TK_FUNID,
    TK_FIELDID,
    TK_RUID,

    TK_NUM,
    TK_RNUM,

    TK_ERROR,
    TK_COMMENT,
    TK_EOF

} tokenType;

typedef struct {

    tokenType token;
    char lexeme[MAX_LEXEME_LEN];
    int lineNo;

    union {
        int intVal;
        float realVal;
    } value;

} tokenInfo;

#endif